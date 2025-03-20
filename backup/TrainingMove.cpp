#include <iostream>
#include <JetsonGPIO.h>
//#include <pair>
//#include <windows.h>
#include <unistd.h>
#include <pthread.h>
#include "CommApi.h"
#include "proxy/ProxyMotion.h"
#include <fstream>
#include <istream>
#include <vector>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
//extern "C"{
#include "tty_uart_RW.h"
//}
// 拍照和保存流量计用到的头文件
// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <cstring>
// #include <sys/socket.h>
// #include <time.h>

//#define Rd_length 44
using namespace std;

vector<string> S_point(10, "1");
fstream file_txt;
vector<double> pre_coordinate(4);
int cow_nums;

Hsc3::Comm::CommApi cmApi("");
Hsc3::Proxy::ProxyMotion pMot(&cmApi);

// 存储奶杯坐标，框左右点各一个
vector<double> Point_glass1{0.0, 0.0, 0.0};
vector<double> Point_glass2{0.0, 0.0, 0.0};
vector<double> Point_glass3{0.0, 0.0, 0.0};
vector<double> Point_glass4{0.0, 0.0, 0.0};
// 存储药浴坐标
vector<double> Point_med1{0.0, 0.0, 0.0};
vector<double> Point_med2{0.0, 0.0, 0.0};
vector<double> Point_med3{0.0, 0.0, 0.0};
vector<double> Point_med4{0.0, 0.0, 0.0};
// 存储乳刷坐标
vector<double> Point_brush1{0.0, 0.0, 0.0};
vector<double> Point_brush2{0.0, 0.0, 0.0};
vector<double> Point_brush3{0.0, 0.0, 0.0};
vector<double> Point_brush4{0.0, 0.0, 0.0};

//机械臂运动函数声明
bool connectIPC(Hsc3::Comm::CommApi &cmApi, std::string strIP, uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi &cmApi);
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, double axis4, double axis5, bool isJoint);
void loadPosData(GeneralPos &generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint);
void waitDone(Hsc3::Proxy::ProxyMotion &pMot);

void *startpul(void* arg);
void *startrun(void *arg);
void *startsend(void *arg);
/*
    read_split函数说明：
        read_split1: 读取文件第1行的数据
        read_split4: 读取文件第2行的数据
        read_split5: 读取文件第3行的数据
        read_split6: 读取文件第4行的数据
        read_split2: 统计文件的行数
        read_split3: 读取文件所有数据
*/
vector<double> &read_split1(vector<double> &pre_coordinate, fstream &file);
int read_split2(fstream &file);
vector<vector<double>> &read_split3(vector<vector<double>> &A, fstream &file);
vector<double> &read_split4(vector<double> &pre_coordinate, fstream &file);
vector<double> &read_split5(vector<double> &pre_coordinate, fstream &file);
vector<double> &read_split6(vector<double> &pre_coordinate, fstream &file);
// 四个套奶杯的函数
vector<double> &glass_point1(const vector<double> a, vector<double> &b);
vector<double> &glass_point2(const vector<double> a, vector<double> &b);
vector<double> &glass_point3(const vector<double> a, vector<double> &b);
vector<double> &glass_point4(const vector<double> a, vector<double> &b);
// 药浴坐标函数
vector<double> &med_point(const vector<double> a, vector<double> &b);
// 乳刷坐标函数
vector<double> &brush_point(const vector<double> a, vector<double> &b);
// 获取编号和拍照函数
void *start_detect_id(void *arg);
void *get_detect_id(void *arg);
char *get_ID();
void take_photo_c(char *tag);
void save_curnum(char *tag);

// 装载点位声明
GeneralPos targetPos0, targetPos1, targetPos2, targetPos3, targetPos4, targetPos5, targetPos6, targetPos7, targetPos8, targetPos9;
GeneralPos targetPos11, targetPos12, targetPos13, targetPos14, targetPos01, targetPos02, targetPos03, targetPos04, targetPos19, testpoint;
GeneralPos targetPosa,targetPosb,targetPosc;
// 药浴的装载点位声明
GeneralPos targetPos001,targetPos002,targetPos003,targetPos004;
// 旋转的机械臂中间位置，总原点是111 套杯原点是333
GeneralPos targetPos111,targetPos222,targetPos333,targetPos444,targetPos555;
// 乳刷坐标声明
GeneralPos targetPosbrush1,targetPosbrush2,targetPosbrush3,targetPosbrush4;
GeneralPos targetPosbrush01,targetPosbrush02,targetPosbrush03,targetPosbrush04;

// 一共创建8个线程
pthread_t npid[12];
// flowid为每个奶杯的编号
int flowid1=0;
int flowid2=1;
int flowid3=2;
int flowid4=3;

// 读取编号用到的全局变量
int inner_cow_id = 0;
bool stop_write = false;
bool start_read_id = false;
pthread_t pid[2];

// 存储坐标的文件的路径
std::string Binocular_camera_path = "/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt";

//vacuum为真空
vector<int> vacuum = {17, 27, 22, 18};
//pul为脉动
vector<int> pul = {23, 19, 26, 20};
//拉绳
vector<int> cord = {12, 16, 6, 13};
//立杯
vector<int> cup = {25, 9, 11, 8};

//收发标致位
static vector<int> flowflag(4,1); // flowflag = {1, 1, 1, 1}
char mybuf[2048];
char mybuf1[2048];

void *my_test(void* arg){
	char my_testa;
	while(cin>>my_testa){
		pMot.stopJog(0);
		cout<<"ready to stop"<<endl;
	}
}

void *toptest(void* arg) {
	int fd =libtty_open((char *)"/dev/ttyUSB0");
	int fd1 =libtty_open((char *)"/dev/ttyUSB1");
	if(fd<0 || fd1<0){
		cout<<"libtty_read error"<<endl;
		pthread_exit(NULL);
	}
	int ret=libtty_setopt(fd,921600,8,1,'n',0);
	int ret1=libtty_setopt(fd1,921600,8,1,'n',0);
	if(ret!=0 || ret1!=0){
		cout<<"libtty_read error"<<endl;
	}
	uint8_t wbuf[]={0x57,0x10,0xFF,0xFF,0x00,0xFF,0xFF,0x63};
	while(1){
		memset(mybuf,0,2048);
		memset(mybuf1,0,2048);
		write(fd,wbuf,sizeof(wbuf));
		write(fd1,wbuf,sizeof(wbuf));
		ret = read(fd,mybuf,sizeof(mybuf));
		ret1 = read(fd1,mybuf1,sizeof(mybuf1));
		cout<<"ret: "<<ret<<endl;
		cout<<"ret1: "<<ret1<<endl;
		for(int i=0;i<ret;i++){
			printf(" 0x%.2x",(uint8_t)mybuf[i]);
		}
		cout<<"--------------------"<<endl;
		for(int i=0;i<ret1;i++){
			printf(" 0x%.2x",(uint8_t)mybuf1[i]);
		}
		cout<<"--------------------"<<endl;
		for(int i=9;i<ret;i=i+6){
			int32_t temp=(int32_t)((uint8_t)mybuf[i]<<8|(uint8_t)mybuf[i+1]<<16|(uint8_t)mybuf[i+2]<<24)/256;
			float result=temp/1000.0f;
			cout<<result<<" ";
			if(result>5 &&result<150) {
				cout<<"ready to stop"<<endl;
				pMot.stopJog(0);
				pthread_exit(NULL);
			}
		}
		for(int i=9;i<ret1;i=i+6){
			int32_t temp1=(int32_t)((uint8_t)mybuf1[i]<<8|(uint8_t)mybuf1[i+1]<<16|(uint8_t)mybuf1[i+2]<<24)/256;
			float result1=temp1/1000.0f;
			cout<<result1<<" ";
			if(result1>5 &&result1<150) {
				cout<<"ready to stop"<<endl;
				pMot.stopJog(0);
				pthread_exit(NULL);
			}
		}
		cout<<"--------------"<<endl;
		sleep(1);
	}
	return (void*)0;

}


int main()
{
	signal(SIGINT, sig_handler);
	GPIO::setmode(GPIO::BCM);

	//*
	verbose = 0;  // 串口详细打印于否;
	//	for(int init_idx=0;init_idx<4;init_idx++){
	//		flag_setZero=5;
	//		while(flag_setZero){
	//			startsend((void*)&init_idx);
	//			//flag_setZero=0;			
	//		}
	//		cout<<" # " << init_idx <<" SetZero end!" <<endl;
	//	} // */

	/*	//test
		verbose = 0;
		sleep(15);
		for(int init_idx=0;init_idx<4;init_idx++){
		npid[0+init_idx*3]=pthread_create(&npid[0+init_idx*3],NULL,startrun,(void*)&init_idx);
		if(npid[0]!=0){
		perror("pthread_create");
		}
		sleep(1);
		npid[1+init_idx*3]=pthread_create(&npid[1+init_idx*3],NULL,startsend,(void*)&init_idx);
		if(npid[1]!=0){
		perror("pthread_create");
		}
		sleep(1);
	//npid[2+init_idx*3]=pthread_create(&npid[2+init_idx*3],NULL,startpul,(void*)&init_idx);
	//	if(npid[2]!=0){
	//		perror("pthread_create");
	//	}
	//sleep(1);
	}
	while(1){
	sleep(2);
	for(int i=0;i<4;i++){
	cout<<"flowflag"<<i+1<<" : "<<flowflag[i]<<endl;
	}
	if(flowflag[0]==0 && flowflag[1]==0 && flowflag[2]==0 && flowflag[3]==0) break;
	}
	cout<<"流量都归零了，准备回到起始点"<<endl;
	sleep(10000);
	// */   

	//	Hsc3::Comm::CommApi cmApi("");
	//	Hsc3::Proxy::ProxyMotion pMot(&cmApi);

	loadPosData(targetPosa, -22, 80, -6, 32, false);
	loadPosData(targetPos0, 25, -55, 118, 58, true); // 机械臂总原点
	loadPosData(targetPos9, 0, 0, 0, 25, false);	 // 奶杯原点，只定义了，实际没有参与运动

	// 首先运动到标定原点(targetPos0)进行图像识别
	if (connectIPC(cmApi, "10.10.56.214", 23234))
	{
		pMot.setOpMode(OP_T1);
		//	pMot.setWorkFrame(0, FRAME_JOINT);
		pMot.setWorkFrame(0, FRAME_BASE); // FRAME_BASE: 用户坐标系
		pMot.setWorkpieceNum(0, -1);	  // setWorkpieceNum(int8_t gpId, int8_t num): gpId 组号，num 工件坐标索引(0..15)
		// 机器人上使能
		pMot.setGpEn(0, true);
		sleep(1);
		pMot.moveTo(0, targetPos0, false); // true: 直线运动, false: 关节运动
		// pMot.moveTo(0, targetPosa, true);
		waitDone(pMot);
		cout << "已经运动到关节坐标系设置的原点" << endl;
	}
	else
	{
		cout << "连接失败" << endl;
	}

	// --------------------- 拍照 ---------------------
	char *id = get_ID();
	std::cout << "id: " << id << std::endl;
	take_photo_c(id);
	std::cout << "已拍照" << std::endl;
	// ------------------------------------------------
	// ----------------- 保存流量数据 ------------------
	// 不应该在这里保存，应在最后完成挤奶后保存
	save_curnum(id);
	std::cout << "已保存流量计数据" << std::endl;
	// ------------------------------------------------
	// std::cout << "测试完毕!" << std::endl;
	// sleep(1000);

	cout << "开始设置gpio" << endl;
	// 设置BCM模式
	GPIO::setmode(GPIO::BCM);
	// 1-4是拉紧绳子的，5-8是将奶杯撑起来的
	int out_put1 = 12; // 32
	int out_put2 = 16; // 36
	int out_put3 = 6;  // 31
	int out_put4 = 13; // 33
	int out_put5 = 25; // 22
	int out_put6 = 9;  // 21
	int out_put7 = 11; // 23
	int out_put8 = 8;  // 24
	// 9/10/11为乳刷/药浴/支架
	int out_put9 = 24;	// 18
	int out_put10 = 10; // 19
	int out_put11 = 7;	// 26
	// 以下为控制真空
	int out_put13 = 17; // 11,真空杯组1
	int out_put14 = 27; // 13,真空杯组2
	int out_put15 = 22; // 15,真空杯组3
	int out_put16 = 18; // 12，真空杯组4
	// 以下为控制脉动
	int out_put17 = 23; // 16,脉动1
	int out_put18 = 19; // 35,脉动2
	int out_put19 = 26; // 37,脉动3
	int out_put20 = 20; // 38，脉动4
	cout << "gpio设置完毕" << endl;

label:
	// 设置引脚为输出引脚并初始化为低电平
	GPIO::setup(out_put1, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put2, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put3, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put4, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put5, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put6, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put7, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put8, GPIO::OUT, GPIO::LOW);
	// 乳刷/药浴
	GPIO::setup(out_put9, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put10, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put11, GPIO::OUT, GPIO::LOW);
	// 真空杯组
	GPIO::setup(out_put13, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put14, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put15, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put16, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put17, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put18, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put19, GPIO::OUT, GPIO::LOW);
	GPIO::setup(out_put20, GPIO::OUT, GPIO::LOW);

	cout << "gpio初始化完毕" << endl;

	if (connectIPC(cmApi, "10.10.56.214", 23234)) {
		pMot.setOpMode(OP_T1);
		//	pMot.setWorkFrame(0, FRAME_JOINT);
		pMot.setWorkFrame(0, FRAME_BASE);
		pMot.setWorkpieceNum(0, -1);
		//机器人上使能
		pMot.setGpEn(0, true);
		sleep(1);

		cout << "begin to test" << endl;
		pthread_t mypid;
		mypid = pthread_create(&mypid, NULL, my_test, NULL);
		loadPosData(targetPosb, 0, -50, -6, 32, false);
		loadPosData(targetPosc, 0, -50, -6, 32, false);
		sleep(1);
		pMot.moveTo(0, targetPosb, true); // true 直线 false 关节
		sleep(3);
		cout << "get to b again" << endl;
		pMot.moveTo(0, targetPosa, true); // true 直线 false 关节
		waitDone(pMot);
		sleep(1);
		pMot.moveTo(0, targetPosb, true); // true 直线 false 关节
		waitDone(pMot);
		sleep(100);

		loadPosData(targetPos111, 25, -45, 118, 70, true);	// true 关节 先转4轴防止撞到
		loadPosData(targetPos222, -30, -45, 118, 80, true); // true 关节再转前两周到达平行位置

		pMot.moveTo(0, targetPos111, false); // true 直线 false 关节
		waitDone(pMot);
		sleep(1);
		pMot.moveTo(0, targetPos222, false);
		waitDone(pMot);
		sleep(1);

		pMot.setWorkFrame(0, FRAME_BASE);
		pMot.setWorkpieceNum(0, 2);
		sleep(1);

		loadPosData(targetPos333, 0, 0, -10, -10, false);//false 直线
		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);

		cout << "准备开始药浴和套杯" << endl;
		sleep(5);

		cout << "开始药浴" << endl;
		// 首先进行药浴，读取文档，并将坐标保存至数组中(Point_med, Point_brush ...)
		vector<vector<double>> med_A;
		// 把Binocular_camera.txt里存储的数据读取到med_A中，med_A为二维浮点数数组
		read_split3(med_A, file_txt);
		std::cout << "已读取坐标" << std::endl; // 测试

		/*
			med_point(const vector<double> a, vector<double>& b), brush_point(...)函数：
				计算坐标，用a中的数据进行计算，结果保存到b中
		*/
		// med_A[i]: vector<double>
		// vector<double> Point_med1{0.0, 0.0, 0.0};
		med_point(med_A[0], Point_med1);
		med_point(med_A[1], Point_med2);
		med_point(med_A[2], Point_med3);
		med_point(med_A[3], Point_med4);

		brush_point(med_A[0], Point_brush1);
		brush_point(med_A[1], Point_brush2);
		brush_point(med_A[2], Point_brush3);
		brush_point(med_A[3], Point_brush4);

		// targetPos00x 形式的是药浴的装载点位
		loadPosData(targetPos001, Point_med1[0], Point_med1[1], -15, -10, false);
		loadPosData(targetPos002, Point_med2[0], Point_med2[1], -15, -10, false);
		loadPosData(targetPos003, Point_med3[0], Point_med3[1], -15, -10, false);
		loadPosData(targetPos004, Point_med4[0], Point_med4[1], -15, -10, false);

        // targetPosbrush 形式的是乳刷坐标
		loadPosData(targetPosbrush1, Point_brush1[0], Point_brush1[1], -15, -10, false);
		loadPosData(targetPosbrush2, Point_brush2[0], Point_brush2[1], -15, -10, false);
		loadPosData(targetPosbrush3, Point_brush3[0], Point_brush3[1], -15, -10, false);
		loadPosData(targetPosbrush4, Point_brush4[0], Point_brush4[1], -15, -10, false);
		loadPosData(targetPosbrush01, Point_brush1[0], Point_brush1[1], 35, -10, false);
		loadPosData(targetPosbrush02, Point_brush2[0], Point_brush2[1], 35, -10, false);
		loadPosData(targetPosbrush03, Point_brush3[0], Point_brush3[1], 35, -10, false);
		loadPosData(targetPosbrush04, Point_brush4[0], Point_brush4[1], 35, -10, false);

		pMot.moveTo(0, targetPos001, true);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::HIGH);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::LOW);
		waitDone(pMot);
		sleep(1);

		pMot.moveTo(0, targetPos002, true);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::HIGH);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::LOW);
		waitDone(pMot);
		sleep(1);

		pMot.moveTo(0, targetPos003, true);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::HIGH);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::LOW);
		waitDone(pMot);
		sleep(1);

		pMot.moveTo(0, targetPos004, true);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::HIGH);
		waitDone(pMot);
		GPIO::output(out_put10, GPIO::LOW);
		waitDone(pMot);
		sleep(1);

		//套杯原点是targetPos333 loadPosData(targetPos333, 0, 0, -10, -10, false);
		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);
		sleep(1);

		loadPosData(targetPos555, 300, 0, -10, -10, false);
		pMot.moveTo(0, targetPos555, true);
		waitDone(pMot);
		sleep(1);

		// 测试
		// sleep(1000);

		// 滚刷和支架
		GPIO::output(out_put9, GPIO::HIGH);
		waitDone(pMot);
		GPIO::output(out_put11, GPIO::HIGH);
		waitDone(pMot);
		sleep(2);

		pMot.moveTo(0, targetPosbrush1, true);
		waitDone(pMot);
		pMot.moveTo(0, targetPosbrush01, true);
		waitDone(pMot);
		sleep(5);

		pMot.moveTo(0, targetPosbrush2, true);
		waitDone(pMot);
		pMot.moveTo(0, targetPosbrush02, true);
		waitDone(pMot);
		sleep(5);

		pMot.moveTo(0, targetPosbrush3, true);
		waitDone(pMot);
		pMot.moveTo(0, targetPosbrush03, true);
		waitDone(pMot);
		sleep(5);

		pMot.moveTo(0, targetPosbrush4, true);
		waitDone(pMot);
		pMot.moveTo(0, targetPosbrush04, true);
		waitDone(pMot);
		sleep(5);

		pMot.moveTo(0, targetPos555, true);
		waitDone(pMot);
		sleep(3);

		GPIO::output(out_put9, GPIO::LOW);
		waitDone(pMot);
		GPIO::output(out_put11, GPIO::LOW);
		waitDone(pMot);
		sleep(3);

		cout << "滚刷完成" << endl;

		// 进行套杯动作
		// 每次套完杯以后，启动脉动和真空
		// ------------------------- 1号乳头套杯 -------------------------
		cout << "开始套杯" << endl;
		pMot.moveTo(0, targetPos333, true); // 套杯原点是targetPos333
		waitDone(pMot);
		sleep(1);
		cout << "已回到套杯起始点" << endl;

		// 读取文件并将坐标装载
		// int timer1 = 0;
		while (1) {
			// timer1++;
			// if(timer1==50000) goto label;
			cow_nums = read_split2(file_txt);
			if (cow_nums != 4)
				cout << "此次未识别到四个乳头" << endl;
			if (cow_nums == 4) {
				cout << "此次识别到了四个乳头" << endl;
				read_split1(pre_coordinate, file_txt); // 读取第一行数据
				break;
			}
		}
		// 一号乳头第一次套杯
		glass_point1(pre_coordinate, Point_glass4);
		loadPosData(targetPos01, Point_glass4[0], Point_glass4[1], -10, -10, false);
		loadPosData(targetPos11, Point_glass4[0], Point_glass4[1], Point_glass4[2], -10, false);
		
		pMot.moveTo(0, targetPos01, true);
		waitDone(pMot);
		GPIO::output(out_put5, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos11, true);
		waitDone(pMot);
		sleep(1);
		GPIO::output(out_put1, GPIO::HIGH);
		// 此时对准乳头了，启动真空和脉动
		// 启动脉动需要调用子线程
		GPIO::output(out_put13, GPIO::HIGH);

		npid[0] = pthread_create(&npid[0], NULL, startrun, (void *)&flowid1);
		if (npid[0] != 0) {
			perror("pthread_create");
		}
		npid[1] = pthread_create(&npid[1], NULL, startsend, (void *)&flowid1);
		if (npid[1] != 0) {
			perror("pthread_create");
		}
		npid[2] = pthread_create(&npid[2], NULL, startpul, (void *)&flowid1);
		if (npid[2] != 0) {
			perror("pthread_create");
		}

		GPIO::output(out_put1, GPIO::HIGH);
		waitDone(pMot);

		pMot.moveTo(0, targetPos333, true); // targetPos333是套杯原点
		waitDone(pMot);
		sleep(2);

		cow_nums = read_split2(file_txt);
		if (cow_nums > 3) {
			cout << "1号乳头第一次套杯失败，进行第二次" << endl;
			// 说明第一次套杯失败，需要进行第二次套杯
			// 一号乳头第二次套杯
			GPIO::output(out_put1, GPIO::LOW);
			GPIO::output(out_put5, GPIO::HIGH);
			pMot.moveTo(0, targetPos01, true);
			waitDone(pMot);
			GPIO::output(out_put5, GPIO::HIGH);
			waitDone(pMot);
			pMot.moveTo(0, targetPos11, true);
			waitDone(pMot);
			GPIO::output(out_put1, GPIO::HIGH);
			waitDone(pMot);
			sleep(1);
			pMot.moveTo(0, targetPos333, true);
			waitDone(pMot);
			sleep(1);
		}

		cow_nums = read_split2(file_txt);
		if (cow_nums == 3) {
			cout << "一号套杯完成" << endl;
		}
		else {
			cout << "一号乳头套杯失败，进行二号乳头套杯" << endl;
		}

		// ------------------------- 2号乳头套杯 -------------------------
		// 回到原点，准备进行二号乳头套杯
		// 此时机械臂位于targetPos333处
		sleep(1);
		// int timer2 = 0;
		while (1) {
			// timer2++;
			// if(timer2==1000000) goto label;
			cow_nums = read_split2(file_txt);
			cout << "cow_nums:" << cow_nums;
			if (cow_nums < 3)
				cout << "此次未识别到三个乳头" << endl;
			else if (cow_nums == 3) {
				// 识别到了3个乳头(第一次套杯成功，1号乳头识别不到)，读取第一个乳头的坐标(第一个指文件第一行)
				read_split1(pre_coordinate, file_txt);
				break;
			}
			else if (cow_nums == 4) {
				// 识别到了4个乳头(第一次套杯失败，仍能识别到1号乳头)，此时读取2号乳头坐标，read_split4()负责读取文件第二行
				read_split4(pre_coordinate, file_txt);
				cout << pre_coordinate[0] << ',' << pre_coordinate[1] << ',' << pre_coordinate[2] << endl;
				break;
			}
		}

		glass_point2(pre_coordinate, Point_glass2);
		loadPosData(targetPos02, Point_glass2[0], Point_glass2[1], -10, -10, false);
		loadPosData(targetPos12, Point_glass2[0], Point_glass2[1], Point_glass2[2], -10, false);
		pMot.moveTo(0, targetPos02, true);
		waitDone(pMot);
		GPIO::output(out_put6, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos12, true);
		waitDone(pMot);
		// 此时对准乳头了，启动真空和脉动
		// 启动脉动需要调用子线程
		GPIO::output(out_put14, GPIO::HIGH);
		npid[3] = pthread_create(&npid[3], NULL, startrun, (void *)&flowid2);
		if (npid[3] != 0) {
			perror("pthread_create");
		}
		npid[4] = pthread_create(&npid[4], NULL, startsend, (void *)&flowid2);
		if (npid[4] != 0) {
			perror("pthread_create");
		}
		npid[5] = pthread_create(&npid[5], NULL, startpul, (void *)&flowid2);
		if (npid[5] != 0) {
			perror("pthread_create");
		}
		GPIO::output(out_put2, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);
		sleep(2);

		cow_nums = read_split2(file_txt);
		if (cow_nums > 2) {
			// 说明2号乳头套杯失败，再来一次
			// 2号乳头二次套杯
			cout << "2号乳头第一次套杯失败，进行第二次" << endl;
			GPIO::output(out_put2, GPIO::LOW);
			GPIO::output(out_put6, GPIO::HIGH);
			pMot.moveTo(0, targetPos02, true);
			waitDone(pMot);
			GPIO::output(out_put6, GPIO::HIGH);
			waitDone(pMot);
			pMot.moveTo(0, targetPos12, true);
			waitDone(pMot);
			GPIO::output(out_put2, GPIO::HIGH);
			waitDone(pMot);
			sleep(1);
			pMot.moveTo(0, targetPos333, true);
			waitDone(pMot);
			sleep(1);
		}
		cow_nums = read_split2(file_txt);
		if (cow_nums == 2)
			cout << "二号套杯完成" << endl;
		else
			cout << "二号乳头套杯失败，进行三号乳头套杯" << endl;

		// ------------------------- 3号乳头套杯 -------------------------
		// 回到原点(targetPos333)，准备进行三号乳头套杯
		sleep(1);
		// int timer3 = 0;
		while (1) {
			// timer3++;
			// if(timer3==1000000) goto label;
			cow_nums = read_split2(file_txt);
			cout << "cow_nums:" << cow_nums;
			if (cow_nums < 2)
				cout << "此次未识别到两个乳头" << endl;
			else if (cow_nums == 2) {
				// 识别到2个乳头，说明第一次和第二次套杯都成功了，此时直接读取文件第1行数据就是3号乳头的坐标
				read_split1(pre_coordinate, file_txt);
				break;
			}
			else if (cow_nums == 3) {
				// 识别到3个乳头，说明第一次或者第二次套杯有一次失败，此时读取文件第2行数据就是3号乳头的坐标
				read_split4(pre_coordinate, file_txt);
				cout << pre_coordinate[0] << ',' << pre_coordinate[1] << ',' << pre_coordinate[2] << endl;
				break;
			}
			else if (cow_nums == 4) {
				// 识别到4个乳头，说明第一次和第二次套杯都失败了，此时调用read_split5()读取文件第3行数据，即3号乳头的坐标
				read_split5(pre_coordinate, file_txt);
				break;
			}
		}

		glass_point3(pre_coordinate, Point_glass3);
		loadPosData(targetPos04, Point_glass3[0], Point_glass3[1], -10, -10, false);
		// loadPosData(targetPos03, Point_glass3[0], Point_glass3[1], -10, -10, false);
		loadPosData(targetPos14, Point_glass3[0], Point_glass3[1], Point_glass3[2], -10, false);
		// loadPosData(targetPos13, Point_glass3[0], Point_glass3[1], Point_glass3[2], -10, false);
		pMot.moveTo(0, targetPos04, true);
		// pMot.moveTo(0, targetPos03, true);
		waitDone(pMot);
		GPIO::output(out_put7, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos14, true);
		// pMot.moveTo(0, targetPos13, true);
		waitDone(pMot);
		// 此时对准乳头了，启动真空和脉动
		// 启动脉动需要调用子线程
		GPIO::output(out_put15, GPIO::HIGH);
		npid[6] = pthread_create(&npid[6], NULL, startrun, (void *)&flowid3);
		if (npid[6] != 0) {
			perror("pthread_create");
		}
		npid[7] = pthread_create(&npid[7], NULL, startsend, (void *)&flowid3);
		if (npid[7] != 0) {
			perror("pthread_create");
		}
		npid[8] = pthread_create(&npid[8], NULL, startpul, (void *)&flowid3);
		if (npid[8] != 0) {
			perror("pthread_create");
		}
		GPIO::output(out_put3, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);
		sleep(2);

		cow_nums = read_split2(file_txt);
		if (cow_nums > 1) {
			// 说明3号乳头套杯失败，再来一次
			// 3号乳头二次套杯
			cout << "3号乳头第一次套杯失败，进行第二次" << endl;
			GPIO::output(out_put3, GPIO::LOW);
			GPIO::output(out_put7, GPIO::HIGH);
			pMot.moveTo(0, targetPos04, true);
			// pMot.moveTo(0, targetPos03, true);
			waitDone(pMot);
			GPIO::output(out_put7, GPIO::HIGH);
			waitDone(pMot);
			pMot.moveTo(0, targetPos14, true);
			// pMot.moveTo(0, targetPos13, true);
			waitDone(pMot);
			GPIO::output(out_put3, GPIO::HIGH);
			waitDone(pMot);
			sleep(1);
			pMot.moveTo(0, targetPos333, true);
			waitDone(pMot);
			sleep(1);
		}
		cow_nums = read_split2(file_txt);
		if (cow_nums == 1)
			cout << "三号套杯完成" << endl;
		else
			cout << "三号乳头套杯失败，进行四号乳头套杯" << endl;

		// ------------------------- 4号乳头套杯 -------------------------
		// 回到原点，准备进行4号乳头套杯
		sleep(1);
		// int timer4 = 0;
		while (1) {
			// timer4++;
			// if(timer4==500000) goto label;
			cow_nums = read_split2(file_txt);
			if (cow_nums < 1)
				cout << "此次未识别到一个乳头" << endl;
			else if (cow_nums == 1) {
				// 识别到1个乳头，说明前三次套杯都成功了，此时直接读取文件第1行数据就是4号乳头的坐标
				read_split1(pre_coordinate, file_txt);
				break;
			}
			else if (cow_nums == 2) {
				// 识别到2个乳头，说明前三次套杯其中有一次失败，此时读取文件第2行数据就是4号乳头的坐标
				read_split4(pre_coordinate, file_txt);
				break;
			}
			else if (cow_nums == 3) {
				// 识别到3个乳头，说明前三次套杯其中有两次失败，此时读取文件第3行数据就是4号乳头的坐标
				read_split5(pre_coordinate, file_txt);
				break;
			}
			else if (cow_nums == 4) {
				// 识别到4个乳头，说明前四次套杯全部失败，此时读取文件第4行数据就是4号乳头的坐标
				read_split6(pre_coordinate, file_txt);
				break;
			}
		}

		glass_point4(pre_coordinate, Point_glass1);
		loadPosData(targetPos03, Point_glass1[0], Point_glass1[1], -10, -10, false);
		// loadPosData(targetPos04, Point_glass1[0], Point_glass1[1], -10, -10, false);
		loadPosData(targetPos13, Point_glass1[0], Point_glass1[1], Point_glass1[2], -10, false);
		// loadPosData(targetPos14, Point_glass1[0], Point_glass1[1], Point_glass1[2], -10, false);
		pMot.moveTo(0, targetPos03, true);
		// pMot.moveTo(0, targetPos04, true);
		waitDone(pMot);
		GPIO::output(out_put8, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos13, true);
		// pMot.moveTo(0, targetPos14, true);
		waitDone(pMot);
		sleep(1);
		// 此时对准乳头了，启动真空和脉动
		// 启动脉动需要调用子线程
		GPIO::output(out_put16, GPIO::HIGH);
		npid[9] = pthread_create(&npid[9], NULL, startrun, (void *)&flowid4);
		if (npid[9] != 0) {
			perror("pthread_create");
		}
		npid[10] = pthread_create(&npid[10], NULL, startsend, (void *)&flowid4);
		if (npid[10] != 0) {
			perror("pthread_create");
		}
		npid[11] = pthread_create(&npid[11], NULL, startpul, (void *)&flowid4);
		if (npid[11] != 0) {
			perror("pthread_create");
		}
		GPIO::output(out_put4, GPIO::HIGH);
		waitDone(pMot);
		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);
		sleep(2);
		
		cow_nums = read_split2(file_txt);
		if (cow_nums > 1) {
			// 说明4号乳头套杯失败，再来一次
			// 4号乳头二次套杯
			cout << "4号乳头第一次套杯失败，进行第二次" << endl;
			GPIO::output(out_put4, GPIO::LOW);
			GPIO::output(out_put8, GPIO::HIGH);
			pMot.moveTo(0, targetPos03, true);
			// pMot.moveTo(0, targetPos04, true);
			waitDone(pMot);
			GPIO::output(out_put8, GPIO::HIGH);
			waitDone(pMot);

			pMot.moveTo(0, targetPos13, true);
			// pMot.moveTo(0, targetPos14, true);
			waitDone(pMot);
			GPIO::output(out_put4, GPIO::HIGH);
			waitDone(pMot);
			sleep(1);
			pMot.moveTo(0, targetPos333, true);
			waitDone(pMot);
			sleep(1);
		}
		cow_nums = read_split2(file_txt);
		if (cow_nums == 0)
			cout << "四号套杯完成" << endl;
		else
			cout << "四号乳头套杯失败" << endl;

		pMot.moveTo(0, targetPos333, true);
		waitDone(pMot);
		cout << "回到套杯起始点，套杯动作完成" << std::endl;
		while (1) {
			sleep(3);
			if (flowflag[0] == 0 && flowflag[1] == 0 && flowflag[2] == 0 && flowflag[3] == 0)
				break;
		}

		// ----------------- 保存流量数据 ------------------
		save_curnum(id);
		// ------------------------------------------------

		cout << "流量都归零了，准备回到起始点" << endl;
		pMot.setWorkFrame(0, FRAME_JOINT);
		pMot.setWorkpieceNum(0, -1);
		sleep(1);
		pMot.moveTo(0, targetPos0, false);
		waitDone(pMot);
		sleep(1000);

		//套杯完成，进行真空吸杯
		/*cout<<"套杯完成，进行真空吸杯"<<endl;
		  int glassnum=4;
		  int flag1=0;
		  int flag2=0;
		  int flag3=0;
		  int flag4=0;
		//测试信号接受*/
	}
	else
	{
		std::cout<<"连接失败"<<std::endl;
	}

	//机器人下使能
	pMot.setGpEn(0, false);
	//断连
	disconnectIPC(cmApi);

	system("pause");
	return 0;
}


void *startpul(void* arg) {
	int a=*(int *)arg; // 强转再解引用，a = flowid1 ~ flowid4 (a = 0, 1, 2, 3)
	cout<<"startpul_begin"<<endl;
	GPIO::setup(pul[a],GPIO::OUT,GPIO::HIGH);
	GPIO::setup(vacuum[a],GPIO::OUT,GPIO::HIGH);
	GPIO::setup(cord[a],GPIO::OUT,GPIO::HIGH);
	GPIO::setup(cup[a],GPIO::OUT,GPIO::HIGH);
	for(int i=0;i<15;i++){
		GPIO::output(pul[a],GPIO::HIGH);
		sleep(1.5);
		GPIO::output(pul[a],GPIO::LOW);
		sleep(1.5);
	}
	while(1){
		GPIO::output(pul[a],GPIO::HIGH);
		int diff=curnum[a]-prenum[a];
		if(flowflag[a]==0){
			break;
		}else if(diff>10){
			sleep(1.5);
			GPIO::output(pul[a],GPIO::LOW);
			sleep(1.5);	
		}else{
			sleep(1);
			GPIO::output(pul[a],GPIO::LOW);
			sleep(1);
		}
	}
	GPIO::output(pul[a],GPIO::LOW);
	GPIO::output(vacuum[a],GPIO::LOW);
	sleep(3);
	GPIO::output(cord[a],GPIO::LOW);
	GPIO::output(cup[a],GPIO::LOW);
	return (void*)0;
}

// int flowid1 = 0;
// pthread_create(&npid[0], NULL, startrun, (void *) &flowid1);
// startrun((void*)&flowid1)
void *startrun(void* arg) {
	int ret=0; 
	//	int equalcnt=0;

	// 打开串口设备文件
	string s="/dev/ttyCH9344USB";
	//cout<<"s"<<s<<endl;
	string fid=to_string(7-*(int*)arg); // 7 - (0, 1, 2, 3); *(int*)arg = 0, fid = "7"
	s= s+fid; // "/dev/ttyCH9344USB7", "/dev/ttyCH9344USB6" ...; s = "/dev/ttyCH9344USB7"
	cout<<"Rdevice: "<< s <<endl;
	int b=*(int*)arg; // b = flowid1 ~ flowid4 (b = 0, 1, 2, 3); b = 0
	int id = 7-*(int*)arg; // id = 7 - (0, 1, 2, 3) = 7, 6, 5, 4; id = 7
	//signal(SIGINT, sig_handler);
	// int fdx[8]={0, 0, 0, 0, 0, 0, 0, 0};
    // fd: 文件描述符; fdx存储文件描述符
	fdx[id] = libtty_open(( char*)s.data()); // fdx[id] = fd; fdx[7] = fd
	if (fdx[id] < 0) {
		printf("libtty_open: %s error.\n", (const char*)s.data());
		pthread_exit(NULL);
	}
	ret = libtty_setopt(fdx[id], speed, 8, 1, 'n', hardflow);
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		pthread_exit(NULL);
	}
	//curnum=0;
	//prenum=-1;
	while(1){
		//cout << "Read in while" <<endl;
		ret = libtty_read(fdx[id],b);
		if (ret < 0)
			printf("libtty_read error: %d\n", ret);
		//cout << "R ret= " << ret <<endl;
		if(ret!=100) continue;

		if(curnum[b]>100 && curnum[b]-prenum[b]< 2 && curnum[b]!=0){
			//if(equalnum[b]==curnum[b])
			//equalcnt[b]++;
			//cout<< equalcnt[b] <<" !: cur_value:"<<dec<<curnum[b]<< "; prenum= " << prenum[b]<<  endl;
			//if(equalcnt[b]==0){				
			cout<<"ready to break"<<endl;
			flowflag[7-id]--;
			//equalcnt[b]=0;
			//break;
			//}

			//equalnum[b]=curnum[b];
		}
		if(curnum[b]>= prenum[b] && curnum[b]-prenum[b]<1000){
			cout << "id: "<< 7-id+1 <<" curnum: "<<dec <<curnum[b]<<"  prenum: "<<prenum[b]<<endl;
			prenum[b]=curnum[b];
		}
		sleep(0.5);
	}
	ret = libtty_close(fdx[id],id);
	if (ret != 0) {
		printf("libtty_close error.\n");
		pthread_exit(NULL);
	}
	return (void*)0;
}

void* startsend(void* arg) {
	int id = *(int*)arg;
	string s="/dev/ttyCH9344USB";
	//cout<<"s"<<s<<endl;
	string fid=to_string(id);
	s= s+fid;
	cout<<"send: "<< s <<endl;
	int ret=0; 

	//signal(SIGINT, sig_handler);
	fdx[id] = libtty_open((char*)s.data());
	if (fdx[id] < 0) {
		printf("libtty_open: %s error.\n", (char*)s.data());
		pthread_exit(NULL);
	}
	ret = libtty_setopt(fdx[id], speed, 8, 1, 'n', hardflow);
	if (ret != 0) {
		printf("libtty_setopt error.\n");
		pthread_exit(NULL);
	}
	if(!flag_setZero){
		cout<<"send: normal" << endl;
		while(1)
		{
			if(flowflag[id]==0) break;
			ret = libtty_write(fdx[id]);
			if (ret <= 0)
				printf("libtty_write error: %d\n", ret);
			sleep(0.5);
		}
	} // /*
	else{
		cout<<"send: setZero" << endl;
		while(1)
		{
			if(!flag_setZero) break;

			ret = libtty_write(fdx[id]);
			if (ret <= 0)
				printf("libtty_write error: %d\n", ret);
			sleep(0.05);
		}  
	}  // */
	ret = libtty_close(fdx[id],id);
	if (ret != 0) {
		printf("libtty_close error.\n");
		pthread_exit(NULL);
	}

	return (void*)0;
}

// read_split1()函数每次只会读取一次文件第一行的数据，把字符串转换成double类型存储到pre_coordinate数组中
// std::fstream file_txt;
// std::vector<double> pre_coordinate(4);
// 调用：read_split1(pre_coordinate, file_txt);
// 读取文件第1行的数据
vector<double>& read_split1(vector<double>& pre_coordinate, fstream& file) {
	cout<<"read_split1"<<endl;
	pre_coordinate.clear();
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	string str;
	getline(file,str);
	cout<<"this result is:"<<endl;
	cout<<str<<endl;
	int j=0;
	for(int i=0;i<str.size();i++){
		if(str[i]==' '){
			pre_coordinate.push_back(stod(str.substr(j, i - j)));
			cout<<stod(str.substr(j, i - j))<<endl;
			j=i+1;
		}
	}
	file.close();
	return pre_coordinate;
}

// read_split2用来统计文件的行数，即识别到了几个乳头
int read_split2(fstream& file) {
	vector<string> A_point(5);
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	int i = 0;
	while (getline(file, A_point[i]))
	{
		i++;
	}
	file.close();
	cout<<"此次识别到"<<i<<"奶头"<<endl;
	file.close();
	return i;
}

// 读取文件中的结果用于药浴和奶刷
// 把路径文件里存储的坐标读取到A中，A是一个4✖3的矩阵，存储这四个乳头对应的xyz坐标
vector<vector<double>>& read_split3(vector<vector<double>>& A, fstream& file) {
labelmed:	
	vector<string> A_point(5);
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	if (!file.is_open())
	{
		cout << "文件读取失败" << endl;
		return A;
	}
	int i = 0;
	while (getline(file, A_point[i]))
	{
		//cout << A[i] << endl;
		cout<<"此次读取成功,第"<<i<<"次"<<endl;
		cout<<"此次读取的坐标为"<<A_point[i]<<endl;
		i++;
	}
	file.close();
	if(i!=4) goto labelmed;
	cout<<"文件读取完成"<<endl;

	// ---------------------------- 保存坐标 ----------------------------
	// time_t t = time(0);
	// char tmp[32] = "";
	// strftime(tmp, sizeof(tmp), "%Y%m%d", localtime(&t));
    // std::string now = tmp;

	std::string id = std::to_string(inner_cow_id);
    // std::string txt_dir = "../../cow_data/" + now + "/coordinate.txt";
	std::string txt_dir = "../../cow_data/coordinate.txt";

	// 打开文件进行读取
	std::ifstream ifs(txt_dir);
	std::string line;
	bool id_exists = false;

	// 检查每一行，看是否已经存在相同的id
	while (std::getline(ifs, line)) {
    	if (line.find(id) != std::string::npos) {
        	id_exists = true;
        	break;
    	}
	}
	ifs.close();

	// 如果id不存在，那么写入数据
	if (!id_exists) {
    	std::ofstream ofs;
    	ofs.open(txt_dir, std::ios::app);
    	ofs << id << ": " << A_point[0] << ", " << A_point[1] << ", " << A_point[2] << ", " << A_point[3] << std::endl;
    	ofs.close();
	}
	// -----------------------------------------------------------------

	// i为四行数据，代表4个乳头；j为xyz三个坐标，共3个
	// 将一个字符串数组A_point转换为一个二维浮点数数组A
	// A: vector<vector<double>> A
	for (string str : A_point) {
		cout << str << endl;
		int j = 0;
		vector<double> this_A;
		for (int i = 0; i < str.size(); i++) {
			if (str[i]==' ') {
				this_A.push_back(stod(str.substr(j, i - j)));
				cout << stod(str.substr(j, i - j)) << endl;
				j = i + 1;
			}
		}
		A.push_back(this_A);
	}

	return A;
}

// 读取文件第2行的数据
// 用于2号乳头的识别读取文件中的结果并进行处理(识别出4个乳头的情况)
vector<double>& read_split4(vector<double>& pre_coordinate, fstream& file) {
	cout<<"read_split1"<<endl;
	vector<string> A_point(5);
	pre_coordinate.clear();
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	string str;
	for(int i=0;i<4;i++){
		getline(file, A_point[i]);
	}
	str=A_point[1];
	cout<<"this result is:"<<endl;
	cout<<str<<endl;
	int j=0;
	for(int i=0;i<str.size();i++){
		if(str[i]==' '){
			pre_coordinate.push_back(stod(str.substr(j, i - j)));
			cout<<stod(str.substr(j, i - j))<<endl;
			j=i+1;
		}
	}
	file.close();
	return pre_coordinate;
}

// 读取文件第3行的数据
// 用于3号乳头的识别读取文件中的结果并进行处理(识别出4个乳头的情况)
vector<double>& read_split5(vector<double>& pre_coordinate, fstream& file) {
	vector<string> A_point(5);
	pre_coordinate.clear();
	cout<<"the test is:"<<endl;
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	string str;
	int i=0;
	while (getline(file, A_point[i]))
	{
		i++;
	}
	str=A_point[2];
	cout<<"this result is:"<<endl;
	cout<<str<<endl;
	int j=0;
	for(int i=0;i<str.size();i++){
		if(str[i]==' '){
			pre_coordinate.push_back(stod(str.substr(j, i - j)));
			cout<<stod(str.substr(j, i - j))<<endl;
			j=i+1;
		}
	}
	file.close();
	return pre_coordinate;
}

// 读取文件第4行的数据
// 用于3号乳头的识别读取文件中的结果并进行处理(识别出4个乳头的情况)
vector<double>& read_split6(vector<double>& pre_coordinate, fstream& file) {
	vector<string> A_point(5);
	pre_coordinate.clear();
	cout<<"the test is:"<<endl;
	//file.open("/home/jx520/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	// file.open("/home/nvidia/Desktop/yolov5_d435i_detection-main/Binocular_camera.txt", ios::in);
	file.open(Binocular_camera_path, ios::in);
	string str;
	int i=0;
	while (getline(file, A_point[i]))
	{
		i++;
	}
	str=A_point[3];
	cout<<"this result is:"<<endl;
	cout<<str<<endl;
	int j=0;
	for(int i=0;i<str.size();i++){
		if(str[i]==' '){
			pre_coordinate.push_back(stod(str.substr(j, i - j)));
			cout<<stod(str.substr(j, i - j))<<endl;
			j=i+1;
		}
	}
	file.close();
	return pre_coordinate;
}

// 计算奶杯坐标，用a中的数据进行计算，结果保存到b中
vector<double>& glass_point1(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] + 56;
	b[1] = a[0] - 33;
	b[2] = 280 - a[2];
	cout << "glass1: " << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

vector<double>& glass_point2(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] - 26;
	b[1] = a[0] - 77;
	b[2] = 250 - a[2];
	cout << "glass2: " << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

vector<double>& glass_point3(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] + 51;
	b[1] = a[0] + 37;
	b[2] = 250 - a[2];
	cout << "glass3: " << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

vector<double>& glass_point4(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] - 20;
	b[1] = a[0] + 62;
	b[2] = 250 - a[2];
	cout << "glass4: " << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

// 计算药浴坐标，用a中的数据进行计算，结果保存到b中
vector<double>& med_point(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] + 50;
	b[1] = a[0];
	b[2] = 220 - a[2];
	cout << "med_point: " << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

// 计算乳刷坐标，用a中的数据进行计算，结果保存到b中
vector<double>& brush_point(const vector<double> a, vector<double>& b) {
	b[0] = - a[1] + 130;
	b[1] = a[0];
	b[2] = 220 - a[2];
	cout << "brush_point:" << b[0] << ", " << b[1] << ", " << b[2] << endl;
	return b;
}

// 进行坐标转换，这个函数没有调用过
void get_glasspoint(vector<vector<double>> coordinate,vector<double>& glass1,vector<double>& glass2,vector<double>& glass3,vector<double>& glass4){
	glass_point1(coordinate[0], glass1);
	glass_point2(coordinate[1], glass2);
	glass_point3(coordinate[2], glass3);
	glass_point4(coordinate[3], glass4);
}

// 识别编号、拍照、保存代码
void *start_detect_id(void *arg) {
	std::string device = "/dev/ttyUSB0";
	int fd, ret, nwrite = 0;

	fd = libtty_open((char*)device.data());
	if (fd < 0) {
		perror("libtty_open error\n");
		pthread_exit(NULL);
	} else {
		printf("opened0!\n");
	}

	ret = libtty_setopt(fd, 4800, 8, 1, 'n', hardflow);
	if (ret != 0) {
		perror("libtty_setopt error\n");
		pthread_exit(NULL);
	} else {
		printf("set0!\n");
	}

	unsigned char buf1[3] = {0xAA, 0x0D, 0xBA};
	unsigned char buf2[8] = {0xAA, 0xB5, 0x13, 0xFF, 0xFF, 0xFF, 0x01, 0x61};
	unsigned char buf3[3] = {0xAA, 0x0A, 0xBD};

	sleep(1);
	nwrite = write(fd, buf1, sizeof(buf1));
	if (nwrite <= 0)
		perror("libtty_write buf1 error\n");
	else
		printf("write buf1!\n");

	sleep(1);
	nwrite = write(fd, buf2, sizeof(buf2));
	if (nwrite <= 0)
		perror("libtty_write buf2 error\n");
	else
		printf("write buf2!\n");

	sleep(1);
    start_read_id = true;
	while (!stop_write) {
		nwrite = write(fd, buf3, sizeof(buf3));
		if (nwrite <= 0)
			perror("libtty_write buf3 error\n");
		else
			printf("write buf3!\n");

		sleep(1);
	}	

	ret = close(fd);
    if (ret != 0) {
        printf("close error\n");
        pthread_exit(NULL);
    } else {
		printf("closed0!\n");
    }

    return (void *) 0;
}

void *get_detect_id(void* arg) {
	std::string device = "/dev/ttyUSB1";
	int fd, ret, nread = 0;

	fd = libtty_open((char*)device.data());
	if (fd < 0) {
		perror("libtty_open error\n");
		pthread_exit(NULL);
	} else {
		printf("opened1!\n");
	}

	ret = libtty_setopt(fd, 4800, 8, 1, 'n', hardflow);
	if (ret != 0) {
		perror("libtty_setopt error\n");
		pthread_exit(NULL);
	} else {
		printf("set1!\n");
	}

	unsigned char buf[50];
	memset(buf, '0', sizeof(buf));

	sleep(1);
	while (true) {
		nread = read(fd, buf, sizeof(buf));
		if (nread == -1) {
        		perror("read error\n");
    	} else {
			// 打印读取到的结果
			printf("read %d bytes: ", nread);
			for (int i = 0; i < nread; i++) {
				printf("%X ", buf[i]);
			}
			printf("\n");

			// start_read_id会在写buf3前置为true，表明可以开始检测奶牛标签了，防止标签过早靠近检测器导致读取出错误的id
			if (start_read_id) {
				if (nread > 40) {  // 含有id信息都会大于40个字节(但不是大于40字节的都含有id信息！)
					stop_write = true; // 停止写入
					break;
				}
			}
		}
		memset(buf, '0', sizeof(buf));
		sleep(1);
	}

	unsigned char raw_id[3];
	memset(raw_id, '0', 3);
	
	// 从读取到的信息中提取出id
	for (int i = 0; i < 50; i++) {
		if (buf[i] == 0x7F) {
			raw_id[0] = buf[i-5];
			raw_id[1] = buf[i-4];
			raw_id[2] = buf[i-3];
			break;
		}
	}

	printf("raw_id: ");
	for (int i = 0; i < 3; i++) {
		printf("%X ", raw_id[i]);
	}
	printf("\n");

	// 二进制取反
    for (int i = 0; i < sizeof(raw_id); i++) {
       	raw_id[i] = ~raw_id[i];
    }

    // 高位置零
	raw_id[0] &= 0x0F;

	// 合并
	int num = 0;
	for (int i = 0; i < 3; i++) {
		num = (num << 8) | raw_id[i];
    }

	/*
		例子：
			{0xEC, 0xBA, 0x93} 的二进制：11101100 10111010 10010011
			先取反：00010011 01000101 01101100 再高4位置零 00000011 01000101 01101100 
			合并成一个int 000000110100010101101100 十进制为：214380
	*/

    printf("转换为十进制后为: %d\n", num);

	inner_cow_id = num;
	// stop_write = true; // 停止写入
	// printf("停止写入\n");

	ret = close(fd);
    if (ret != 0) {
       	printf("close error\n");
       	pthread_exit(NULL);
    } else {
		printf("closed1!\n");
	}

   	return (void *) 0;
}

// 获取奶牛编号
char* get_ID() {
    pid[0] = pthread_create(&pid[0], NULL, start_detect_id, NULL);
	if (pid[0] != 0) {
       	perror("pthread_create error\n");
    } else {
		printf("thread0 created!\n");
	}
	sleep(1);

	pid[1] = pthread_create(&pid[0], NULL, get_detect_id, NULL);
	if (pid[1] != 0) {
        perror("pthread_create error\n");
    } else {
		printf("thread1 created!\n");
	}
	sleep(1);

	// 循环检测，stop_write会在获得奶牛编号(inner_cow_id)后置为true
	while (true)
	{
		if (stop_write) {
			printf("inner_cow_id: %d\n", inner_cow_id);
			break;
		}
	}
	// printf("get_ID()函数的while循环结束\n");

	// int转char数组
    static char cow_id[8];
    sprintf(cow_id, "%d", inner_cow_id);
	
	return cow_id;
}

// 拍照函数
void take_photo_c(char* tag) {
   // 1. 创建客户端，并连接到服务端
    int sock_client = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    connect(sock_client, (sockaddr*)&server_addr, sizeof(sockaddr));

    // 2. 发送数据，并接受服务端数据，tag是奶牛编号
    //char send_info[10] = tag;
    char* send_info = tag;
    send(sock_client, send_info, strlen(send_info) + 1, 0);

    // 3. 关闭客户端
    close(sock_client);
} 

// 保存流量(挤奶量)数据
void save_curnum(char* tag) {
	time_t t = time(0);
	char tmp[32] = "";
	strftime(tmp, sizeof(tmp), "%Y%m%d", localtime(&t));
    std::string now = tmp; // 当前日期，寻找对应的文件夹用

    std::string txt_dir = "../../cow_data/" + now + "/milk_volume.txt"; // 存储文件夹

    std::ofstream ofs;
    ofs.open(txt_dir, std::ios::app);
    ofs << "cow_" << tag << ": " << curnum[0] << " " << curnum[1] << " " << curnum[2] << " " << curnum[3] << std::endl;
    // ofs << "cow_" << tag << ": " << curnum[0] << " " << curnum[1] << " " << curnum[2] << " " << curnum[3] << " " << std::endl;
    ofs.close();
}

/************************************************************************/
/*            
 * @brief 连接IPC
 * @param cmApi:通信客户端对象，建议在自定义的函数中如果需要传递客户端对象，都使用引用传递。
 * @param strIP:IP，控制器默认IP是"10.10.56.214"
 * @param uPort:端口号,固定端口号:23234
 */
/************************************************************************/
bool connectIPC(Hsc3::Comm::CommApi & cmApi, std::string strIP,uint16_t uPort) {
	//1.设置非自动重连模式,连接前调用。
	cmApi.setAutoConn(false);
	//2.连接
	Hsc3::Comm::HMCErrCode ret = cmApi.connect(strIP,uPort);
	if (ret!=0)
	{
		printf("CommApi::connect() : ret = %lu\n",ret);
	}
	//3.查询是否连接
	if (cmApi.isConnected())
	{
		std::cout<<"连接成功"<<std::endl;
		return true;
	}
	else
	{
		std::cout<<"连接失败"<<std::endl;
		return false;
	}
}

/************************************************************************/
/* 
 * @brief 断开与IPC的连接
 * @param cmApi:通信客户端对象
 */
/************************************************************************/
bool disconnectIPC(Hsc3::Comm::CommApi & cmApi) {
	Hsc3::Comm::HMCErrCode ret=cmApi.disconnect();
	sleep(5);
	if (cmApi.isConnected())
	{
		return false;
	}
	else
	{
		return true;
	}
}

/************************************************************************/
/* 
 * @brief 装载数据
 */
/************************************************************************/
void loadPosData(GeneralPos & generalPos, double axis0, double axis1, double axis2, double axis3, double axis4, double axis5, bool isJoint) {
	generalPos.config=1048576;
	generalPos.isJoint=isJoint;
	generalPos.ufNum=-1;
	generalPos.utNum=-1;
	generalPos.vecPos.push_back(axis0);
	generalPos.vecPos.push_back(axis1);
	generalPos.vecPos.push_back(axis2);
	generalPos.vecPos.push_back(axis3);
	generalPos.vecPos.push_back(axis4);
	generalPos.vecPos.push_back(axis5);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
}

// isJoint: 是否关节点，和moveTo()函数的第3个参数(isLinear: 是否直线运动、关节运动)有关
// isJoint是true，则isLinear为false(是关节点，则关节运动)；isJoint是false，则isJoint为true(不是关节点，则直线运动)
void loadPosData(GeneralPos& generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint) {
	generalPos.config = 1048576;
	generalPos.isJoint = isJoint; // 是否关节点
	generalPos.ufNum= 0; // 工件号(修改为对应的用户坐标系的工件坐标)
	generalPos.utNum=-1; // 工具号
	generalPos.vecPos.push_back(axis0);
	generalPos.vecPos.push_back(axis1);
	generalPos.vecPos.push_back(axis2);
	generalPos.vecPos.push_back(axis3);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
	generalPos.vecPos.push_back(0);
}

/************************************************************************/
/* 
 * @brief 等待运动停止，只能用于手动模式下运动到点，用于检测是否处于静止或错误状态
 * @param pMot:运动功能代理
 */
/************************************************************************/
void waitDone(Hsc3::Proxy::ProxyMotion & pMot) {
	ManState manualState = MAN_STATE_MAX;

	//此延时是为了防止机器人还未进入运动状态
	sleep(1);

	while(1)
	{
		pMot.getManualStat(manualState);
		if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR)
		{
			if (manualState == MAN_STATE_ERROR)
			{
				std::cout<<"错误"<<std::endl;
			}
			break;
		}
	}
}