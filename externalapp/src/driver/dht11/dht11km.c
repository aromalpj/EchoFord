/* dht11km.c - DHT11 Device Driver */

/*
 *
 * Copyright (c) 2025 Trenser Technology Solutions
 *
 * The right to copy, distribute, modify or otherwise make use of
 * this software may be licensed only to the terms of an applicable
 * Trenser Software Solutions.
 */

/*
modification history
---------------------
11Nov25,akg  Code Refactoring of DHT11 Driver with respect to D0-178C standard
*/

/*
DESCRIPTION
This file provides the device driver for reading values from DHT11 temperature
and humidity sensor.

 By default the DHT11 is connected to GPIO pin 0 (pin 3 on the GPIO connector)
 The Major version default is 80 but can be set via the command line.
 Command line parameters: gpio_pin=X - a valid GPIO pin value
 					      driverno=X - value for Major driver number
 						  format=X - format of the output from the sensor

\is
 Usage:
         Load driver: 	insmod ./dht11km.ko <optional variables>
 				i.e.   	insmod ./dht11km.ko gpio_pin=2 format=3

 		  Set up device file to read from (i.e.):
 						mknod /dev/dht11 c 80 0
 						mknod /dev/myfile c <driverno> 0    -  to set the output
                        to your own file and driver number

 		  To read the values from the sensor: cat /dev/dht11
\ie

INCLUDE FILES: module.h, errno.h, init.h, io.h, interrupt.h, ioport.h, kernel.h,
time.h, string.h, types.h, wait.h, mm.h, delay.h, platform_device.h, irq.h
syscore_ops.h, fcntl.h, spinlock.h, timekeeping.h, fs.h, uaccess.h

*/

/* includes */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <linux/irq.h>
#include <linux/fcntl.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/* defines */

#define DHT11_DRIVER_NAME 		 "dht11"
#define SUCCESS 				  0
#define FAILURE 				 -1
#define BUF_LEN 				 80
#define DHT11_TOTAL_BITS         40
#define DHT11_MAX_PULSE_US   	 77
#define DHT11_MIN_PULSE_US   	 15
#define DHT11_THRESHOLD_PULSE_US 60
#define DHT11_BITS_PER_BYTE      8
#define DHT11_TOTAL_BYTES        5
#define GPIO_BASE 				 0x3F200000
#define INTERRUPT_GPIO0 		 17

/* globals */

/* set GPIO pin g as input */

#define GPIO_DIR_INPUT(g) 		*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))

/* set GPIO pin g as output */

#define GPIO_DIR_OUTPUT(g) 		*(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

/* get logical value from gpio pin g */

#define GPIO_READ_PIN(g) 		(*(gpio+13) & (1<<(g))) && 1

/* sets bits which are 1 ignores bits which are 0 */

#define GPIO_SET_PIN(g)			*(gpio+7) = 1<<g;

/* clears bits which are 1 ignores bits which are 0 */
#define GPIO_CLEAR_PIN(g) 		*(gpio+10) = 1<<g;

/* clear GPIO interrupt on the pin we use */
#define GPIO_INT_CLEAR(g) 		*(gpio+16) = (*(gpio+16) | (1<<g));

/* GPREN0 GPIO Pin Rising Edge Detect Enable/Disable */
#define GPIO_INT_RISING(g,v) 		\
                    *(gpio+19) = v ? (*(gpio+19) | (1<<g)) : (*(gpio+19) ^ (1<<g))

/* GPFEN0 GPIO Pin Falling Edge Detect Enable/Disable */
#define GPIO_INT_FALLING(g,v) 		\
                    *(gpio+22) = v ? (*(gpio+22) | (1<<g)) : (*(gpio+22) ^ (1<<g))

/* forward declarations */

static int 		readDht11(struct inode *, struct file *);
static int 		closeDht11(struct inode *, struct file *);
static ssize_t	deviceRead(struct file *, char *, size_t, loff_t *);
static void 	clearInterrupts(void);

/* locals */

static struct timespec64 lasttv = {0, 0};               /* last timespec value*/
static uint32_t 		 deviceOpen = 0;				/* Used to prevent multiple access to device */
static char 			 msg[BUF_LEN] = {0};			/* The msg given to userspace */
static char *            pMsgPtr;
static spinlock_t 		 lock;							/* Locking for critical section */
static unsigned int 	 bitCount = 0U;
static unsigned int 	 byteCount = 0U;
static uint32_t       	 started = 0U;					/* Indicate if read started or not */
static unsigned char 	 dht[5] = {0};					/* Store result bytes */
static int 				 format = 0;					/* Default result format */
static uint32_t 		 gpioPin = 0;					/* Default GPIO pin */
static int 				 driverNumber = 80;				/* Default driver number */
volatile uint32_t *		 gpio;                          /* Pointer to hardware register */

/* Possible valid GPIO pins */

uint32_t 				 validGpioPins[] = { 0, 1, 4, 8, 7, 9, 10, 11, 14, 15,
                                                    17, 18, 21, 22, 23,24, 25 };

/* Operations that can be performed on the device */

static struct file_operations fops =
    {
    .read = deviceRead,
    .open = readDht11,
    .release = closeDht11
    };

/*******************************************************************************
 * dht11InitModule - Initialize the DHT11 driver module
 *
 * This function initializes the DHT11 sensor driver, checks for a valid GPIO pin,
 * registers the character device, and initializes GPIO memory for communication.
 *
 * This function returns -EINVAL when:
 * - An invalid GPIO pin is specified.
 *
 * This function returns non-zero negative value when:
 * - Registering the DHT11 character device fails.
 * - GPIO memory initialization fails.
 *
 * PARAMETERS: N/A
 *
 * GLOBALS:
 * \is
 * \i <gpioPin>
 *     [in] uint32_t [0 - UINT32_MAX]  -- GPIO pin number
 *
 * \ie
 * RETURNS:
 * \is
 * \i <0>
 *    Successful initialization of DHT11 driver.
 * \i <-EINVAL>
 *    When an invalid GPIO pin is specified.
 * \i <result>
 *    Non zero negative value returned by register_chrdev [result] or initPort
 *    upon failure.
 * \ie
 */

static int __init dht11InitModule
    (
    void
    )
    {
                                                     /* req: dht11InitModule_LLR_1 */

    int32_t result = 0;
    uint32_t gpioValid = 0U;
    uint32_t index = 0U;

    /* Check for valid gpio pin number */

    for(index = 0U; i < ARRAY_SIZE(validGpioPins); i++)
        {
        if(gpioPin == validGpioPins[i])
            {
            gpioValid = 1;
            break;
            }
        }

    if (!gpioValid)
        {
        printk(KERN_ERR DHT11_DRIVER_NAME ": invalid GPIO pin specified!\n");
        return -EINVAL;
        }
                                                    /* req: dht11InitModule_LLR_2 */

    result = register_chrdev(driverNumber, DHT11_DRIVER_NAME, &fops);

    if (result < 0)
        {
        printk(KERN_ALERT DHT11_DRIVER_NAME " Registering dht11 driver failed with %d\n", result);
        return result;
        }

    printk(KERN_INFO DHT11_DRIVER_NAME ": driver registered!\n");

                                                    /* req: dht11InitModule_LLR_3 */

    result = initPort();

    if (result < 0)
        {
        printk(KERN_ERR DHT11_DRIVER_NAME ": GPIO memory initialization failed!\n");
        return result;
        }

    return 0;
    }

/*******************************************************************************
 * irqHandler - DHT11 sensor GPIO interrupt handler
 *
 * IRQ handler for the DHT11 sensor GPIO pin. Measures pulse timing and
 * extracts sensor data bits from high/low pulse durations.
 *
 * This function returns IRQ_HANDLED when:
 * - when [data] greater than [DHT11_TOTAL_BITS] AND [signal] is set to 1
 * - when [data] is less than [DHT11_MIN_PULSE_US]
 * - when [data] is greater than [DHT11_THRESHOLD_PULSE_US]
 *
 * PARAMETERS:
 * \is
 * \i <irq>
 *    UNUSED Parameter
 * \i dev_id
 *    UNUSED Parameter
 * \ie
 *
 * GLOBALS:
 * \is
 * \i <started>
 *   [out] uint32_t  [0 - UINT32_MAX] -- Indicate if read started or not
 *
 * \i <dht[]>
 *   [out] unsigned char [0 - UINT8_MAX] -- Store result bytes
 *
 * \i <bitCount>
 *   [out] unsigned int [0 - UINT64_MAX] -- Tracks bit position
 *
 * \i <byteCount>
 *   [out] unsigned int [0 - UINT64_MAX] -- Counts the number of bytes
 *
 * \i <gpioPin>
 *   [in] uint32_t [0 - UINT32_MAX]  -- GPIO Pin Number
 *
 * \i <lasttv>
 *   [out] [Valid Pointer] -- A pointer to a struct timespec64
 *
 * \ie
 * RETURNS:
 * \is
 * \i  <IRQ_HANDLED>
 *  When [data] greater than [DHT11_TOTAL_BITS] AND [signal] is set to 1
 *
 * \i <IRQ_HANDLED>
 *  When [data] is less than [DHT11_MIN_PULSE_US]
 *
 * \i <IRQ_HANDLED>
 *  When [data] is greater than [DHT11_THRESHOLD_PULSE_US]
 * \ie
 *
 */

static irqreturn_t irqHandler
    (
    int irq,   		    /* UNUSED PARAMETER */
    void * devId		/* UNUSED PARAMETER */
    )
    {
                                                       /* req: irqHandler_LLR_1 */

    static struct timespec64 lastts = {0, 0};
    struct 		  timespec64 tv;                       /* Current timestamp */
    uint64_t 	  data = 0U;
    uint64_t 	  signal = 0U;
    s64 		  deltvNs;

    /* use the GPIO signal level */

    signal = GPIO_READ_PIN(gpioPin);

    /* reset interrupt */

    GPIO_INT_CLEAR(gpioPin);

    /* get current time */

    ktime_get_real_ts64(&tv);

    /* get time since last interrupt in microseconds */

    deltvNs = (tv.tv_sec - lastts.tv_sec) * 1000000000LL + (tv.tv_nsec - lastts.tv_nsec);
    do_div(deltvNs, 1000);
    data = (uint64_t)deltvNs;  					 	  /* quotient after division */
    lasttv = tv;								      /* Save last interrupt time  */

    if((signal == 1U) && (data > DHT11_TOTAL_BITS))
        {
        started = 1U;
        return IRQ_HANDLED;
        }
                                                       /* req: irqHandler_LLR_2*/

    if((signal == 0U) && (started == 1U))
        {
        if(data > DHT11_MAX_PULSE_US)
            {
            return IRQ_HANDLED;						/* Start/spurious? signal */
            }

        if(data < DHT11_MIN_PULSE_US)
            {
            return IRQ_HANDLED;						/* Spurious signal? */
            }

        if (data > DHT11_THRESHOLD_PULSE_US)
            {
            dht[byteCount] |= (0x80 >> bitCount);	/* Add a 1 to the data byte */
            }

        bitCount++;

        if(bitCount == DHT11_BITS_PER_BYTE)
            {
            bitCount = 0U;
            byteCount++;
            }
        }

    return IRQ_HANDLED;
    }

/*******************************************************************************
 * setupInterrupts - Configures DHT11 sensor GPIO interrupt
 *
 * Initializes and configures hardware interrupts for the DHT11 sensor GPIO pin.
 * Registers the interrupt handler, enables edge detection
 *
 * This function manages the following:
 * - Registers IRQ handler.
 * - Enables rising/falling edge detection for pulse measurement.
 *
 * This function returns [-EBUSY]
 * - when <result> [request_irq] returns -EBUSY
 *
 * This function returns [-EINVAL]
 * - when interrupt setup successfully
 *
 * PARAMETERS: N/A
 *
 * GLOBALS:
 * \is
 * \i <lock>
 *    [in] spinlock_t -- Locking for critical section
 * \i <gpioPin>
 *    [in] uint32_t -- Default GPIO pin
 * \ie
 *
 * RETURNS:
 * \is
 * \i <0>
 *  Successful IRQ setup and configuration
 * \i <-EBUSY>
 *  IRQ already in use
 * \i <-EINVAL>
 *  Invalid IRQ number or handler registration failed
 * \ie
 */

static int setupInterrupts
    (
    void    /* No parameters*/
    )
    {
                                                /* req: setupInterrupts_LLR_1 */

    int32_t result = 0;
    unsigned long flags;

    result = request_irq(INTERRUPT_GPIO0, irqHandler, 0, DHT11_DRIVER_NAME, (void*) gpio);

    switch (result)
        {
        case -EBUSY:
            printk(KERN_ERR DHT11_DRIVER_NAME ": IRQ %d is busy\n", INTERRUPT_GPIO0);
            return -EBUSY;

        case -EINVAL:
            printk(KERN_ERR DHT11_DRIVER_NAME ": Bad irq number or handler\n");
            return -EINVAL;

        default:
            printk(KERN_INFO DHT11_DRIVER_NAME	": Interrupt %04x obtained\n", INTERRUPT_GPIO0);
            break;
        }
                                                /* req: setupInterrupts_LLR_2 */

    spin_lock_irqsave(&lock, flags);

    /* GPREN0 GPIO Pin Rising Edge Detect Enable */

    GPIO_INT_RISING(gpioPin, 1);

    /* GPFEN0 GPIO Pin Falling Edge Detect Enable */
    
    GPIO_INT_FALLING(gpioPin, 1);

    /* Clear interrupt flag */

    GPIO_INT_CLEAR(gpioPin);

    spin_unlock_irqrestore(&lock, flags);

    return 0;
    }

/*******************************************************************************
 * initPort - Reserve and remap GPIO memory region for DHT11 driver
 *
 * This function reserves the GPIO memory region and performs the necessary
 * memory remapping for GPIO access by the DHT11 driver.
 *
 * This function returns -EBUSY when:
 * - <request_mem_region> returns NULL as the GPIO memory region cannot be reserved.
 * - <ioremap> returns NULL as the GPIO memory region cannot be mapped into
 *    virtual address space.
 *
 * PARAMETERS: N/A
 *
 * GLOBALS:
 * \is
 * \i <gpio>
 *  - [Valid pointer] Mapped gpio pointer.
 * \ie
 *
 *
 * RETURNS:
 * \is
 * \i <0>
 *    Successful reservation and mapping of GPIO memory.
 * \i <-EBUSY>
 *    When the GPIO memory region cannot be reserved and mapped.
 * \ie
 */

static int initPort
    (
    void    /* No parameters*/
    )
    {
                                                /* req: initPort_LLR_1 */

    /* reserve GPIO memory region */

    if (request_mem_region(GPIO_BASE, SZ_4K, DHT11_DRIVER_NAME) == NULL)
        {
        printk(KERN_ERR DHT11_DRIVER_NAME ": unable to obtain GPIO I/O memory address\n");
        return -EBUSY;
        }

    /* remap the GPIO memory */

    gpio = ioremap(GPIO_BASE, SZ_4K);

    if (gpio == NULL)
        {
        printk(KERN_ERR DHT11_DRIVER_NAME ": failed to map GPIO I/O memory\n");
        return -EBUSY;
        }

    return 0;
    }

/*******************************************************************************
 * dht11ExitModule - Clean up resources and unregister DHT11 driver
 *
 * This function releases any reserved or mapped GPIO memory, cleans up
 * remaining driver resources, and unregisters the DHT11 character device.
 *
 * Errors are logged if unregistration fails.
 *
 * PARAMETERS: N/A
 *
 * GLOBALS:
 * \is
 * \i <gpio>
 *  - [Valid pointer] Mapped gpio pointer.
 * \ie
 *
 *
 * RETURNS: None
 *
 * ERRORS:
 *
 */

static void __exit dht11ExitModule
    (
    void  /* No parameters*/
    )
    {
                                                /* req: dht11ExitModule_LLR_1 */

    int32_t result = 0;

    /* Release mapped memory and allocated region */

    if(gpio != NULL)
        {
        (void)iounmap(gpio);
        (void)release_mem_region(GPIO_BASE, SZ_4K);
        printk(DHT11_DRIVER_NAME ": cleaned up resources\n");
        }
                                                /* req: dht11ExitModule_LLR_2 */

    /* Unregister the driver */

    result = unregister_chrdev(driverNumber, DHT11_DRIVER_NAME);

    if (result < 0)
        {
        printk(DHT11_DRIVER_NAME ": Unregistration failed\n");
        }

    printk(DHT11_DRIVER_NAME ": cleaned up module\n");

    }

/*******************************************************************************
 * readDht11 - Read data from the DHT11 sensor via character device
 *
 * This function is called when a process attempts to read from the DHT11 device
 * It applies retry logic, interacts with the GPIO to trigger and fetch the
 * sensor's data, validates checksum, and formats the result.
 * The function provides output in multiple formats.
 *
 * This function returns -EINVAL when:
 *  - <pInode> is NULL
 *  - <pFile> is NULL
 *
 *  This function returns -EBUSY when:
 *  - <deviceOpen> is not zero
 *
 *  This function returns -FAILURE when:
 *  - <setupInterrupts> fails
 *
 *  This function returns SUCCESS when:
 *  - When formated data has been read
 *
 * PARAMETERS:
 * \is
 * \i pInode
 *    [in] struct inode * -- UNUSED PARAMETER
 * \i pFile
 *    [in] struct file * -- UNUSED PARAMETER
 * \ie
 *
 * GLOBALS:
 * \is
 * \i <deviceOpen>
 *    [in,out] uint32_t [0 - UINT32_MAX] - Flag used to prevent multiple access to device
 * \i <started>
 * 	  [in] uint32_t [0 - UINT32_MAX] - Indicates if read has started
 * \i <byteCount>
 * 	  [in] unsigned int [0 - UINT64_MAX] - Tracks byte count
 * \i <bitCount>
 * 	   [in] unsigned int [0 - UINT64_MAX] - Tracks bit position in byte
 * \i <dht>
 * 	   [in,out] unsigned char [0 - UINT8_MAX] - Stores sensor result bytes
 * \i <gpioPin>
 * 	   [in] uint32_t [0 - UINT32_MAX] - GPIO pin number
 * \i <lasttv>
 * 	   [in] int64_t [0 - INT64_MAX] - To store last timespec value
 * \i <msg>
 * 	   [out] char [0 - INT8_MAX] - Message buffer
 * \ie
 * RETURNS:
 * \is
 * \i <SUCCESS>
 *    Valid data from the DHT11 sensor has been read, formatted, and returned.
 * \i <-EINVAL>
 *    The <inode> or <pFile> file pointer supplied is NULL.
 * \i <-EBUSY>
 *    The <deviceOpen> device is already open.
 * \i <FAILURE>
 *    An error occurred during interrupt setup.
 * \ie
 */

static int readDht11
    (
    struct inode *pInode,      /* UNUSED PARAMETER */
    struct file *pFile		   /* UNUSED PARAMETER */
    )
    {
                                                /* req: readDht11_LLR_1 */
    char result[5] = {0};
    uint32_t retry = 0U;
    uint32_t hum22 = 0U;         				/* DHT22 humidity in 0.1 % RH */
    uint32_t temp22 = 0U;						/* DHT22 temperature in 0.1 degrees C */
    uint32_t valid = 0U;         				/* Flag to indicate if a valid result is obtained */
    int32_t  intr = 0;

    if (pInode == NULL || pFile == NULL)
        {
        return -EINVAL;
        }

    if (deviceOpen)
        {
        return -EBUSY;
        }

                                                /* req: readDht11_LLR_2 */

    try_module_get(THIS_MODULE); 				/*Increase use count*/

    deviceOpen++;

    /* Attempt read operation with retry logic */

    while (retry < 5U && !valid)
        {
        started = 0U;
        bitCount = 0U;
        byteCount = 0U;
        memset(dht, 0, sizeof(dht));

        GPIO_DIR_OUTPUT(gpioPin); 				/* Set pin to output */
        GPIO_CLEAR_PIN(gpioPin);  				/* Set low */
        mdelay(20);                				/* DHT11 needs min 18mS to signal a startup */
        GPIO_SET_PIN(gpioPin);    				/* Take pin high */
        udelay(40);                				/* Stay high for a bit before swapping to read mode */
        GPIO_DIR_INPUT(gpioPin);  				/* Change to read */

        /* Start timer to time pulse length */

        (void) ktime_get_real_ts64(&lasttv);

        /* Set up interrupts */

        intr = setupInterrupts();

        if (intr != 0)
            {
            printk (DHT11_DRIVER_NAME ": Setting up Interrupts failed \n");

            return FAILURE;
            }
                                                /* req: readDht11_LLR_3 */

        /* Give the dht11 time to reply */

        mdelay(100);

       /* Check if the read results are valid. If not then try again! */

        if (((unsigned char)(dht[0] + dht[1] + dht[2] + dht[3]) == dht[4]) && (byteCount == 5))
            {
            sprintf(result, "OK");
            valid = 1U;
            } else
            {
            sprintf(result, "BAD");
            (void) clearInterrupts();
            mdelay(2100);  						/* Can only read from sensor every 1 second
                                                    so give it time to recover */
            retry++;
            }
        }

    /* Return the result in various different formats */

    switch(format)
        {
        case 0:
            sprintf(msg, "Values: %u, %u, %u, %u, %u, %s\n", dht[0], dht[1],
                                                dht[2], dht[3], dht[4], result);

            break;

        case 1:

            sprintf(msg, "%0X,%0X,%0X,%0X,%0X,%s\n", dht[0], dht[1], dht[2],
                                                        dht[3], dht[4], result);

            break;

        case 2:

            sprintf(msg, "%02X,%02X,%02X,%02X,%02X,%s\n", dht[0], dht[1], dht[2],
                                                        dht[3], dht[4], result);

            break;

        case 3: 										/* DHT 11 readout */

            sprintf(msg, "Temperature: %uC\nHumidity: %u\nResult:%s\n", dht[2],
                                                                dht[0], result);
            break;

        case 4: 										/* DHT22 readout */

            temp22 = (dht[2] & 0x7F) * 256 + dht[3];

            if (dht[2] & 0x80)
            {
                temp22 = -(temp22);
            }

            hum22 = dht[0] * 256 + dht[1];

            sprintf(msg, "Temperature: %u.%uC\nHumidity: %u.%u\nResult:%s\n",
                            temp22/10, temp22%10, hum22/10, hum22%10, result);

            break;

        default:

            msg[0] = '\0'; 							/* Handle unexpected format value */

            break;

        }

    pMsgPtr = msg;

    return SUCCESS;
    }


/*******************************************************************************
 * closeDht11 - Closes the device file.
 *
 * This function decrements deviceOpen count. Clears all the resource
 *
 * This function returns [0] on success.
 *
 * PARAMETERS:
 * \is
 * \i pInode
 *    UNUSED parameter
 * \i pFile
 *    UNUSED parameter
 * \ie
 *
 * GLOBALS:
 * \is
 * \i deviceOpen
 * 	  [out] uint32_t [0 - UINT32_MAX] -- Device usage count
 * \ie
 *
 * ERRORS: None
 *
 * RETURNS:
 * \is
 * \i <0>
 *    Returns 0 if successfully clear the resources
 * \ie
 */

static int closeDht11
    (
    struct inode *pInode,     /* UNUSED Parameter*/
    struct file *pFile        /* UNUSED Parameter*/
    )
    {

                                                      /* req: closeDht11_LLR_1 */

    module_put(THIS_MODULE);

    /* Decrement the usage count, or else once you opened the file, you'll never
       get get rid of the module. */

    deviceOpen--;

    (void)clearInterrupts(void);

    return 0;
    }

/*******************************************************************************
 * clearInterrupts - Clear GPIO edge-detect interrupts and free IRQ
 *
 * This function disables both rising and falling edge detection on the configured
 * GPIO pin. It safely acquires and releases the spinlock to protect critical
 * sections and then frees the associated interrupt line for the DHT11 device.
 *
 * This function does not return a value.
 *
 * PARAMETERS: N/A
 *
 * GLOBALS:
 * \is
 * \i <lock>
 *    [in]  [0 - UINT32_MAX] -- Uses the global spinlock
 * \i <gpioPin>
 *    [in]  [0 - UINT32_MAX] -- GPIO pin configuration
 * \i <gpio>
 *  - [Valid pointer] -- Mapped gpio pointer.
 * \ie
 *
 * ERRORS:  N/A
 *
 * RETURNS:  N/A
 */

static void clearInterrupts
    (
    void		/* No parameters*/
    )
    {
                                                     /* req: clearInterrupts_LLR_1 */

    unsigned long flags;

    spin_lock_irqsave(&lock, flags);

    /* GPREN0 GPIO Pin Rising Edge Detect Disable */

    GPIO_INT_RISING(gpioPin, 0);

    /* GPFEN0 GPIO Pin Falling Edge Detect Disable */

    GPIO_INT_FALLING(gpioPin, 0);

    spin_unlock_irqrestore(&lock, flags);

    free_irq(INTERRUPT_GPIO0, (void *) gpio);
    }

/*******************************************************************************
 * deviceRead -  Read data from kernel space to user space.
 *
 *  This function [deviceRead] copies the data from the kernel data segment to
 *  the user data segment.
 *  This function returns 0 when
 *  - <pMsgPtr> is NULL
 *  - <pMsgPtr> value is 0
 *  This function returns -EFAULT when:
 *  - <put_user> fails
 *  This function returns -EINVAL when:
 *  - <pFile> is NULL
 *  - <pBuffer> IS NULL
 *  - <pOffset> is NULL
 *  This function returns the number of bytes put into the buffer on successful
 *  copy of data to user space.
 *
 * PARAMETERS:
 * \is
 * \i <pFile>
 *    [in] struct file *. [Valid Pointer] -- The file pointer
 *
 * \i <pBuffer>
 * 	  [out] char *. [Valid Pointer]  -- Buffer to fill with data
 *
 * \i <length>
 * 	  [in] size_t . [0 - UINT64_MAX] -- Length of the buffer
 *
 * \i <pOffset>
 *    [in] size_t . [Valid Pointer]-- Pointer to offset for file position
 * \ie
 *
 * GLOBALS:
 * \is
 * \i <pMsgPtr>
 *  [in] static char. [Valid Pointer] -- Buffer which contains the message data
 * \ie
 *
 * DESIGN GLOBALS: N/A
 *
 * ERRORS: N/A
 *
 * RETURNS:
 * \is
 * \i <bytesRead>
 * When the copy of data to [pBuffer] is successful
 *
 * \i <-EFAULT>
 * When <put_user> fails
 *
 * \i <-EINVAL>
 *  <pFile> is NULL
 *
 * \i <-EINVAL>
 *  <pBuffer> IS NULL
 *
 * \i <-EINVAL>
 *  <pOffset> is NULL
 *
 * \i <0>
 * <pMsgPtr> is NULL
 *
 * \i <0>
 * <*pMsgPtr> is 0
 * \ie
 */

static ssize_t deviceRead
    (
    struct file * pFile,	/* File pointer */
    char * pBuffer,	        /* Buffer to fill with data */
    size_t length,		    /* length of the buffer     */
    loff_t * pOffset		/* pOffset pointer */
    )
    {
                                                     /* req: deviceRead_LLR_1 */

    /* Number of bytes actually written to the buffer */

    uint32_t bytesRead = 0;


    if ((pFile == NULL) || (pBuffer == NULL) || (pOffset == NULL) ||
         (length == 0))
        {
        return -EINVAL;
        }

    /* If we're at the end of the message, return 0 signifying end of file */

    if (pMsgPtr == NULL || *pMsgPtr == 0)
        {
        return 0;
        }
                                                     /* req: deviceRead_LLR_2 */

    /* Actually put the data into the buffer */

    while (length > 0)
        {
        /* The buffer is in the user data segment, not the kernel  segment so
             "*" assignment won't work.  We have to use put_user which copies
             data from the kernel data segment to the user data segment. */

        if (put_user(*pMsgPtr, pBuffer) != 0)
            {
            return -EFAULT;
            }
                                                     /* req: deviceRead_LLR_3 */
        pMsgPtr++;
        pBuffer++;
        length--;
        bytesRead++;
        }

    /* Return the number of bytes put into the buffer */

    return bytesRead;
    }

module_init(dht11InitModule);
module_exit(dht11ExitModule);

MODULE_DESCRIPTION("DHT11 temperature/humidity sensor driver for Raspberry Pi GPIO.");
MODULE_AUTHOR("Nigel Morton");
MODULE_LICENSE("GPL");

/* Command line paramaters for gpio pin and driver major number */

module_param(format, int, S_IRUGO);
MODULE_PARM_DESC(format, "Format of output");
module_param(gpio_pin, int, S_IRUGO);
MODULE_PARM_DESC(gpio_pin, "GPIO pin to use");
module_param(driverno, int, S_IRUGO);
MODULE_PARM_DESC(driverno, "Driver handler major value");
