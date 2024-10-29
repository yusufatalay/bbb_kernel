# BeagleBone Black (debian) Linux Kernel Modules

Series of kernel modules I've write on my BeagleBone Black

## Preparation

I used a BeagleBone Black to develop and test my modules and drivers. To compile them, you need to install the Kernel headers on your BBB.

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install linux-headers-$(uname -r)
sudo apt-get install build-essential
```
