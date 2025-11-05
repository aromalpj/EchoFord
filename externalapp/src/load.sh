#!/bin/sh

# --- Configuration ---
CONFIG_FILE=".config"

# List of packages to ensure are enabled (format: PACKAGE_SYMBOL=y)
# These are Buildroot PACKAGE symbols, not kernel CONFIG symbols.
MODULES_TO_ENABLE="
BR2_PACKAGE_DHT11=y
BR2_PACKAGE_I2C_APP_CLIENT=y
"

echo "### Buildroot/Kernel Module Enabler ###"
echo "Targeting configuration file: $CONFIG_FILE"
echo "----------------------------------------"

if [ ! -f "$CONFIG_FILE" ]; then
	echo "ERROR: Configuration file not found at $CONFIG_FILE."
	echo "Please ensure you have run 'make defconfig' and 'make menuconfig' at least once."
	exit 1
fi

# Function to patch the .config file using sed
enable_module() {
	local SYMBOL_SETTING="$1"
	local SYMBOL=$(echo "$SYMBOL_SETTING" | cut -d'=' -f1)

	# 1. Check if the symbol already exists in the file
	if grep -q "^${SYMBOL}=" "$CONFIG_FILE"; then
		# If it exists, replace the existing definition
		echo "Updating $SYMBOL..."
		sed -i "/^${SYMBOL}=/c\\${SYMBOL_SETTING}" "$CONFIG_FILE"
	else
		# If it doesn't exist, append it to the file
		echo "Adding $SYMBOL..."
		echo "$SYMBOL_SETTING" >> "$CONFIG_FILE"
	fi
}

# --- Main Execution ---
echo "Processing modules..."
echo "$MODULES_TO_ENABLE" | grep -vE '^\s*$|^#' | while IFS= read -r MODULE_LINE; do
	enable_module "$MODULE_LINE"
done

echo "----------------------------------------"
echo "Configuration updated. The packages are now marked for inclusion."
echo "Run 'make' in your Buildroot directory to build the system."
