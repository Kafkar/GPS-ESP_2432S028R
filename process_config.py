import os
import yaml
import json
import hashlib

# Get the project directory (current directory)
project_dir = os.path.dirname(os.path.abspath(__file__))

# Path to the config file
config_path = os.path.join(project_dir, "config.yaml")

# Check if config file exists
if not os.path.isfile(config_path):
    print("Config file not found: {}".format(config_path))
    with open(config_path, 'w') as f:
        yaml.dump({
            'wifi': {
                'ssid': 'YOUR_WIFI_SSID',
                'password': 'YOUR_WIFI_PASSWORD',
                'hostname': 'GPS-ESP32'
            },
            'logger': {
                'server': '192.168.1.100',
                'port': 8080
            }
        }, f, default_flow_style=False)
    print("Created default config file. Please edit it with your settings.")
    exit(1)

# Load the config file
with open(config_path, 'r') as f:
    config = yaml.safe_load(f)

# Convert to JSON
config_json = json.dumps(config)

# Create a header file with the config
config_h_path = os.path.join(project_dir, "src", "config.h")
with open(config_h_path, 'w') as f:
    f.write("#ifndef CONFIG_H\n")
    f.write("#define CONFIG_H\n\n")
    f.write("#include <Arduino.h>\n\n")
    f.write("// Auto-generated config file - DO NOT EDIT\n")
    f.write("// Edit config.yaml instead\n\n")
    f.write("const char* CONFIG_JSON = " + json.dumps(config_json) + ";\n\n")
    f.write("#endif // CONFIG_H\n")

print("Config processed successfully")