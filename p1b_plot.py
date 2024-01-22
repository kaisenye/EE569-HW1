import numpy as np
import matplotlib.pyplot as plt

def read_raw_image(file_path, width, height):
    with open(file_path, 'rb') as file:
        img = np.fromfile(file, dtype=np.uint8, count=width*height)
        img = img.reshape((height, width))
    return img

def plot_histogram(image):
    plt.hist(image.ravel(), bins=256, range=[0,256], color='black')
    plt.title('Histogram')
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Frequency')
    plt.show()

def plot_cumulative_histogram(image):
    hist, bins = np.histogram(image.flatten(), bins=256, range=[0,256])
    cdf = hist.cumsum()
    cdf_normalized = cdf * hist.max() / cdf.max()

    plt.plot(cdf_normalized, color='black')
    plt.title('Cumulative Histogram')
    plt.xlabel('Pixel Intensity')
    plt.ylabel('Cumulative Frequency')
    plt.show()


# file paths
ori_file_path = './images/DimLight.raw'
tfb_file_path = './outputs/tfDimLight.raw'
bf_file_path = './outputs/bfDimLight.raw'

# image dimensions
width = 596  
height = 340   

# plot histogram from orignal DimLight image file
dim_light_ori = read_raw_image(ori_file_path, width, height)
plot_histogram(dim_light_ori)

# plot histogram from transfer-function DimLight image file
dim_light_tf = read_raw_image(tfb_file_path, width, height)
plot_histogram(dim_light_tf)

# plot cumulative histogram from orignal DimLight image file
plot_cumulative_histogram(dim_light_ori)

# plot histogram from bucket filling DimLight image file
dim_light_bf = read_raw_image(bf_file_path, width, height)
plot_histogram(dim_light_bf)
plot_cumulative_histogram(dim_light_bf)

