import os
import sys
# import cv2
import qdarkstyle
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import Qt, QThread, pyqtSignal
from PyQt5.QtGui import QIcon, QPixmap
from PyQt5.QtWidgets import QMessageBox

import detect
from utils.myutil import file_is_pic
# from utils.myutil import file_is_pic, Globals
import save_data
import warnings

warnings.filterwarnings("ignore", category=DeprecationWarning)


class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self, parent=None):
        super(Ui_MainWindow, self).__init__(parent)
        self.timer_detect = QtCore.QTimer()
        self.setupUi(self)
        self.init_logo()  # 初始化logo
        self.init_slots()  # 初始化槽函数
        self.file_path = None  # 数据路径 "../cow_data"
        self.model_path = None  # 模型路径
        self.file_suffix = None  # 文件后缀
        self.result_path = "results"  # 检测图片保存路径
        self.init_file()  # 初始化必要的文件夹

    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(900, 600)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.centralwidget)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setSizeConstraint(QtWidgets.QLayout.SetNoConstraint)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.verticalLayout = QtWidgets.QVBoxLayout()
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)  # 布局的左、上、右、下到窗体边缘的距离
        self.verticalLayout.setObjectName("verticalLayout")

        # 打开图片按钮
        self.pushButton_img = QtWidgets.QPushButton(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.MinimumExpanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_img.sizePolicy().hasHeightForWidth())
        self.pushButton_img.setSizePolicy(sizePolicy)
        self.pushButton_img.setMinimumSize(QtCore.QSize(150, 40))
        self.pushButton_img.setMaximumSize(QtCore.QSize(150, 40))
        font = QtGui.QFont()
        font.setFamily("Agency FB")
        font.setPointSize(14)
        self.pushButton_img.setFont(font)
        self.pushButton_img.setObjectName("pushButton_img")
        self.verticalLayout.addWidget(self.pushButton_img, 0, QtCore.Qt.AlignHCenter)
        self.verticalLayout.addStretch(5)  # 增加垂直盒子内部对象间距

        # 加载模型按钮
        self.pushButton_model = QtWidgets.QPushButton(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_model.sizePolicy().hasHeightForWidth())
        self.pushButton_model.setSizePolicy(sizePolicy)
        self.pushButton_model.setMinimumSize(QtCore.QSize(150, 40))
        self.pushButton_model.setMaximumSize(QtCore.QSize(150, 40))
        self.pushButton_model.setFont(font)
        self.pushButton_model.setObjectName("pushButton_model")
        self.verticalLayout.addWidget(self.pushButton_model, 0, QtCore.Qt.AlignHCenter)
        self.verticalLayout.addStretch(5)

        # 检测按钮
        self.pushButton_detect = QtWidgets.QPushButton(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_detect.sizePolicy().hasHeightForWidth())
        self.pushButton_detect.setSizePolicy(sizePolicy)
        self.pushButton_detect.setMinimumSize(QtCore.QSize(150, 40))
        self.pushButton_detect.setMaximumSize(QtCore.QSize(150, 40))
        self.pushButton_detect.setFont(font)
        self.pushButton_detect.setObjectName("pushButton_detect")
        self.verticalLayout.addWidget(self.pushButton_detect, 0, QtCore.Qt.AlignHCenter)
        self.verticalLayout.addStretch(50)

        # 汇总数据按钮
        self.pushButton_aggregate_data = QtWidgets.QPushButton(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_aggregate_data.sizePolicy().hasHeightForWidth())
        self.pushButton_aggregate_data.setSizePolicy(sizePolicy)
        self.pushButton_aggregate_data.setMinimumSize(QtCore.QSize(150, 40))
        self.pushButton_aggregate_data.setMaximumSize(QtCore.QSize(150, 40))
        self.pushButton_aggregate_data.setFont(font)
        self.pushButton_aggregate_data.setObjectName("pushButton_aggregate_data")
        self.verticalLayout.addWidget(self.pushButton_aggregate_data, 0, QtCore.Qt.AlignHCenter)
        self.verticalLayout.addStretch(50)

        # 显示导出文件夹按钮
        self.pushButton_showdir = QtWidgets.QPushButton(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.pushButton_showdir.sizePolicy().hasHeightForWidth())
        self.pushButton_showdir.setSizePolicy(sizePolicy)
        self.pushButton_showdir.setMinimumSize(QtCore.QSize(150, 50))
        self.pushButton_showdir.setMaximumSize(QtCore.QSize(150, 50))
        self.pushButton_showdir.setFont(font)
        self.pushButton_showdir.setObjectName("pushButton_showdir")
        self.verticalLayout.addWidget(self.pushButton_showdir, 0, QtCore.Qt.AlignHCenter)

        # 右侧图片填充区域
        self.verticalLayout.setStretch(2, 1)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.label = QtWidgets.QLabel(self.centralwidget)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 3)
        self.horizontalLayout_2.addLayout(self.horizontalLayout)
        self.label.setStyleSheet("border: 1px solid white;")  # 添加显示区域边框

        # 底部美化导航条
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 23))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "奶牛乳头评级系统"))
        self.pushButton_img.setText(_translate("MainWindow", "加载数据"))
        self.pushButton_model.setText(_translate("MainWindow", "选择模型"))
        self.pushButton_detect.setText(_translate("MainWindow", "目标检测"))
        self.pushButton_aggregate_data.setText(_translate("MainWindow", "汇总数据"))
        self.pushButton_showdir.setText(_translate("MainWindow", "输出文件夹"))
        self.label.setText(_translate("MainWindow", "TextLabel"))

    # 初始化槽函数
    def init_slots(self):
        self.pushButton_img.clicked.connect(self.load_source)
        self.pushButton_model.clicked.connect(self.select_model)
        self.pushButton_detect.clicked.connect(self.target_detect)
        self.pushButton_showdir.clicked.connect(self.show_dir)
        self.pushButton_aggregate_data.clicked.connect(self.aggregate_data)

    # 绘制LOGO
    def init_logo(self):
        pix = QtGui.QPixmap('')
        self.label.setScaledContents(True)
        self.label.setPixmap(pix)

    # 初始化创建保存文件夹
    def init_file(self):
        if not os.path.exists(self.result_path):
            os.mkdir(self.result_path)

    # 加载图片
    def load_source(self):
        self.file_path, _ = QtWidgets.QFileDialog.getOpenFileName(self, "加载数据", "../cow_data", "All Files(*)")
        self.suffix = self.file_path.split(".")[-1]  # 读取后缀
        pixmap = QPixmap(self.file_path)
        pixmap = pixmap.scaled(self.label.size(), Qt.IgnoreAspectRatio)
        self.label.setPixmap(pixmap)
        if self.file_path != "":
            self.pushButton_img.setText(self.file_path.split('/')[-1])
        # 清空result文件夹内容
        for i in os.listdir(self.result_path):
            file_data = self.result_path + "/" + i
            os.remove(file_data)

    # 选择模型
    def select_model(self):
        model_path = QtWidgets.QFileDialog.getOpenFileName(self, "选择模型", "weights", "Model files(*.pt)")
        self.model_path = model_path
        if model_path[0] != "":
            self.pushButton_model.setText(self.model_path[0].split('/')[-1])

    # 目标检测
    def target_detect(self):
        if self.check_file():
            # 点击之后防止误触，禁用按钮
            self.pushButton_img.setEnabled(False)
            self.pushButton_model.setEnabled(False)
            self.pushButton_detect.setEnabled(False)
            self.pushButton_aggregate_data.setEnabled(False)
            self.thread = DetectionThread(self.file_path, self.model_path, self.label)
            self.thread.start()
            # 子线程运行结束之后signal_done，主线程执行UI更新操作
            # self.thread.signal_done.connect(self.flash_target)
            self.pushButton_img.setEnabled(True)
            self.pushButton_model.setEnabled(True)
            self.pushButton_detect.setEnabled(True)
            self.pushButton_aggregate_data.setEnabled(True)

    # 目标检测之前检查是否选择了数据和模型
    def check_file(self):
        if self.file_path is None or self.file_path == "":
            QMessageBox.information(self, '提示', '请先导入数据')
            return False
        if self.model_path is None or self.model_path[0] == "":
            QMessageBox.information(self, '提示', '请先选择模型')
            return False
        return True

    # 摄像头检测之前检查是否选择了模型
    def check_model(self):
        if self.model_path is None or self.model_path[0] == "":
            QMessageBox.information(self, '提示', '请先选择模型')
            return False
        return True

    # 刷新，没有调用
    def flash_target(self):
        if file_is_pic(self.suffix):
            img_path = os.getcwd() + '/results/' + [f for f in os.listdir('results')][0]
            pixmap = QPixmap(img_path)
            pixmap = pixmap.scaled(self.label.size(), Qt.IgnoreAspectRatio)
            self.label.setPixmap(pixmap)
        # 刷新完之后恢复按钮状态
        self.pushButton_img.setEnabled(True)
        self.pushButton_model.setEnabled(True)
        self.pushButton_detect.setEnabled(True)
        self.pushButton_aggregate_data.setEnabled(True)

    # 显示输出文件夹
    def show_dir(self):
        if self.check_file():
            photo_path = os.path.dirname(self.file_path)
            dir_path = os.path.dirname(photo_path)
            path1 = os.path.join(dir_path, 'results')
            if os.path.isdir(path1):
                path = path1.replace('/', '\\')  # windows专用，linux去掉
                os.system(f"start explorer {path}")  # windows使用
                # os.system("xdg-open " + path1)  # ubuntu使用
            else:
                QMessageBox.information(self, '提示', '请先进行目标检测')
                return False

    # 汇总数据
    def aggregate_data(self):
        if self.check_file():
            temp = os.path.dirname(self.file_path)
            data_path = os.path.dirname(temp)
            csv_path = data_path + '/results/predictions.csv'
            print(csv_path)
            milk_path = data_path + '/milk_volume.txt'
            print(milk_path)

            if os.path.exists(csv_path):
                self.data_thread = AggregateDataThread(csv_path, milk_path)
                self.data_thread.start()
                QMessageBox.information(self, '提示', '已汇总数据')
            else:
                QMessageBox.information(self, '提示', '请先进行目标检测')
                return False


# DetectionThread子线程用来执行导入资源的目标检测
class DetectionThread(QThread):
    signal_done = pyqtSignal(int)  # 是否结束信号

    def __init__(self, file_path, model_path, label):
        super(DetectionThread, self).__init__()
        directory_path = os.path.dirname(file_path)
        self.file_path = directory_path
        self.model_path = model_path
        self.label = label
        # self.project_path = os.path.dirname(directory_path)

    def run(self):
        detect.run(source=self.file_path,
                   weights=self.model_path[0],
                   show_label=self.label, save_img=True,
                   project=os.path.dirname(self.file_path) + '/results')
        self.signal_done.emit(1)  # 发送结束信号


# AggregateDataThread子线程用来汇总数据
class AggregateDataThread(QThread):
    signal_done = pyqtSignal(int)  # 是否结束信号

    def __init__(self, csv_path, milk_path):
        super(AggregateDataThread, self).__init__()
        self.csv_path = csv_path
        self.milk_path = milk_path

    def run(self):
        save_data.get_data(self.csv_path, self.milk_path)
        self.signal_done.emit(1)  # 发送结束信号


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    app.setStyleSheet(qdarkstyle.load_stylesheet_pyqt5())
    ui = Ui_MainWindow()
    # 设置窗口透明度
    # ui.setWindowOpacity(0.93)
    # 去除顶部边框
    # ui.setWindowFlags(Qt.FramelessWindowHint)
    # 设置窗口图标
    icon = QIcon()
    icon.addPixmap(QPixmap("./UI/icon.ico"), QIcon.Normal, QIcon.Off)
    ui.setWindowIcon(icon)
    ui.show()
    sys.exit(app.exec_())
