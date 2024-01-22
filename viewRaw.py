import numpy as np
import matplotlib.pyplot as plt
import os

def read_raw_image(file_path, width, height, color_mode):
    # Open the file
    with open(file_path, 'rb') as file:
        # Read the file and convert to numpy array
        img_data = np.fromfile(file, dtype=np.uint8)
        
        # Reshape the numpy array based on color_mode
        if color_mode == 'gray':
            img_data = img_data.reshape((height, width))
        elif color_mode == 'RGB':
            img_data = img_data.reshape((height, width, 3)) # Assuming the data is in RGB order
        else:
            raise ValueError("Unsupported color mode. Choose 'gray' or 'RGB'")
        
        return img_data

# Prompt user for image details
color_mode = "RGB"
width = 768
height = 512

# Path to your raw image file
# For grayscale use tf_image_path, for RGB use bf_image_path as an example
image_name = input("Enter the path to your raw image name: ")
image_path = "./outputs/" + image_name + ".raw"

# Read and display the image
image = read_raw_image(image_path, width, height, color_mode)
plt.imshow(image, cmap='gray' if color_mode == 'gray' else None)
plt.show()
