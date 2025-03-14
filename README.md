# 自动挤奶机器人
该项目分为两部分，包括牛乳头图像识别系统和机械臂自动套杯系统。图像识别系统使用realsense d435i相机，基于pytorch实现yolov5目标检测，实时返回检测目标相机坐标系下的位置信息；机械臂自动套杯系统实时读取牛乳头在相机坐标系下的位置信息，进行药浴、洗涮乳头、套杯等动作。

## **一、牛乳头图像识别系统**

### 1.Environment：

（1）运行YOLOv5的python环境

进入yolov5_d435i_detection-main722文件夹，输入命令行

```bash
pip install -r requirements.txt
```

（2）安装RealSense SDK

a)  首先从官网下载RealSense SDK源码：[Releases · IntelRealSense/librealsense (github.com)](https://github.com/IntelRealSense/librealsense/releases/)

b)  安装依赖库

```
sudo apt-get install git libssl-dev libusb-1.0-0-dev pkg-config libgtk-3-dev libeigen3-dev 

sudo apt-get install libglfw3-dev
```

c)  编译与安装

进入解压好的源码文件，在新建的build文件夹下进行编译与安装

```
cd librealsense-2.48.0
mkdir build
cd build
cmake ../ -DFORCE_RSUSB_BACKEND=ON -DBUILD_PYTHON_BINDINGS:bool=true -DPYTHON_EXECUTABLE=/usr/bin/python3.6 -DCMAKE_BUILD_TYPE=release -DBUILD_EXAMPLES=true -DBUILD_GRAPHICAL_EXAMPLES=true -DBUILD_WITH_CUDA:bool=true
sudo make uninstall
sudo make clean
make -j4
sudo make install
```

d)  测试

执行`realsense-viewer`如果能正确显示摄像头采集图像界面，则证明SDK库已成功安装

（3）安装pyrealsense2

pyrealsense2是我们在使用python调用摄像头时候会用到的一个包，其实我们前面安装RealSense SDK的时候就已经安装完pyrealsense2了，所以这里只需要输入以下的代码添加到路径中即可。

```
sudo gedit ~/.bashrc

1、export PATH=$PATH:~/.local/bin
2、export PYTHONPATH=$PYTHONPATH:/usr/local/lib
3、export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.6/pyrealsense2

source /.bashrc
```

到这里就已经可以在python3中调用pyrealsense2了，输入以下代码不再提示"`No Mudule Named Pyrealsense2`"错误信息，则证明`pyrealsense2`已安装成功

```
python3
import pyrealsense2
```

至此，牛乳头图像识别系统所需要的环境配置完毕

### 2.Model config：

修改模型配置文件，进入yolov5_d435i_detection-main722\config，打开yolov5s.yaml进行配置

```yaml
# YoloV5模型权重路径
weight:  "weights/best.pt"
# 输入图像的尺寸
input_size: 640
# 类别个数
class_num:  1
# 标签名称
class_name: [ 'cow']
# 阈值设置
threshold:
  iou: 0.45
  confidence: 0.6
# 计算设备
# - cpu
# - 0 <- 使用GPU
device: '0'
```

### 3.Camera config：

分辨率好像只能改特定的参数，不然会报错。d435i可以用 1280x720, 640x480, 848x480。

```python
config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 1280, 720, rs.format.bgr8, 30)
```
### 4.code return xyz：
下方代码实现从像素坐标系到相机坐标系转换，并且标注中心点以及三维坐标信息。
```python
for i in range(len(xyxy_list)):
    ux = int((xyxy_list[i][0]+xyxy_list[i][2])/2)  # 计算像素坐标系的x
    uy = int((xyxy_list[i][1]+xyxy_list[i][3])/2)  # 计算像素坐标系的y
    dis = aligned_depth_frame.get_distance(ux, uy)  
    camera_xyz = rs.rs2_deproject_pixel_to_point(
    depth_intrin, (ux, uy), dis)  # 计算相机坐标系xyz
    camera_xyz = np.round(np.array(camera_xyz), 3)  # 转成3位小数
    camera_xyz = camera_xyz.tolist()
    cv2.circle(canvas, (ux,uy), 4, (255, 255, 255), 5)#标出中心点
    cv2.putText(canvas, str(camera_xyz), (ux+20, uy+10), 0, 1,
                                [225, 255, 255], thickness=2, lineType=cv2.LINE_AA)#标出坐标
    camera_xyz_list.append(camera_xyz)
    #print(camera_xyz_list)
```
### 5.Store coordinates：

下方代码将进行三维变换后的坐标存入Binocular_camera.txt中

```python
filename = 'Binocular_camera.txt'

	with open(filename, 'w') as file_object:

    	if (len(camera_xyz_list) == 4):

	    	file_object.write(str(ste_x3)[0:5] + " " + str(ste_y3)[0:5] + " " + str(ste_z3)[0:5] +" " + "\n")

            file_object.write(str(ste_x1)[0:5] + " " + str(ste_y1)[0:5] + " " + str(ste_z1)[0:5] + " " +"\n")

            file_object.write(str(ste_x2)[0:5] + " " + str(ste_y2)[0:5] + " " + str(ste_z2)[0:5] + " " +"\n")

            file_object.write(str(ste_x4)[0:5] + " " + str(ste_y4)[0:5] + " " + str(ste_z4)[0:5] + " " +"\n")

        elif len(camera_xyz_list) == 3:

            file_object.write(str(ste_x3)[0:5] + " " + str(ste_y3)[0:5] + " " + str(ste_z3)[0:5] + " " +"\n")

            file_object.write(str(ste_x1)[0:5] + " " + str(ste_y1)[0:5] + " " + str(ste_z1)[0:5] + " " +"\n")

            file_object.write(str(ste_x2)[0:5] + " " + str(ste_y2)[0:5] + " " + str(ste_z2)[0:5] +" " + "\n")

        elif len(camera_xyz_list) == 2:

            file_object.write(str(ste_x1)[0:5] + " " + str(ste_y1)[0:5] + " " + str(ste_z1)[0:5] + " " +"\n")

            file_object.write(str(ste_x2)[0:5] + " " + str(ste_y2)[0:5] + " " + str(ste_z2)[0:5] +" " + "\n")

        elif len(camera_xyz_list) == 1:

            file_object.write(str(ste_x1)[0:5] + " " + str(ste_y1)[0:5] + " " + str(ste_z1)[0:5] + " " +"\n")
```

至此，牛乳头图像识别系统已完成。



## **二、机械臂套杯系统**

### 1.Environment：

1、将boost_1_69_0.tar.gz文件复制到英伟达桌面上；

2、在桌面下按住Ctrl+Alt+T打开终端，在终端内输入

```
tar -xzvf boost_1_69_0.tar.gz
```

对复制的压缩包进行解压。解压后会在桌面生成boost_1_69文件，进入文件夹，右键打开终端，输入指令

```
sudo ./bootstrap.sh
```

按下回车键，输入密码nvidia 再次按下回车键，运行完成后继续在终端输入

```
sudo ./b2 install
```

 按下回车键 输入密码nvidia等待运行完，二次开发所需的boost动态库装成功。

3、将文件中的Hsc3DemoLinux文件复制到英伟达控制板桌面上；

4、使用快捷指令Ctrl+Alt+T打开终端，在终端内输入

```
cd /etc/ld.so.conf 
```

按下回车键，继续在终端输入

```
sudo vim hsc.conf
```

 按下回车键，输入密码nvidia 再次按下回车键；进入到vim界面后，首先输入i，之后输入

```
/home/jx520/Desktop/Hsc3DemoLinux/Hsc3Api
```

按下Esc键，输入：再输入wq；退出vim界面之后，在终端输入

```
sudo ldconfig
```

到此二次开发所需的动态库已经添加到相应的路径。

5、打开桌面Hsc3DemoLinux文件夹中的build_Move文件夹，右键打开终端，输入指令make ，指令运行完成之后会在build_Move文件夹中生成Hsc3Demo文件。

### 2.Run：

（1）首先需要进入yolo的文件夹，运行yolo识别程序。

```
cd /home/jx520/Desktop/yolov5_d435i_detection-main
```

运行识别程序

```
python3 rstest719.py
```

（2）当开始识别后，进入机械臂程序的文件夹，启动机械臂程序

```
cd /home/jx520/Desktop/Hsc3DemoLinux/build_Move
```

输入以下代码

```
./Hsc3Demo
```

**至此，完整的运行流程开始启动**
