#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "CommApi.h"
#include "proxy/ProxyMotion.h"

using namespace std;

bool connectIPC(Hsc3::Comm::CommApi& cmApi, std::string strIP, uint16_t uPort);
bool disconnectIPC(Hsc3::Comm::CommApi& cmApi);
void loadPosData(GeneralPos& generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint);
void waitDone(Hsc3::Proxy::ProxyMotion& pMot);
int countLines(fstream& file);
void readPos(vector<double>& pre_coordinate, fstream& file_txt, int index);

std::string Binocular_camera_path = "/home/enqi/cow_robot/yolov5_d435i_detection/Binocular_camera.txt";
fstream file_txt;
vector<double> pre_coordinate(3);

int main() {
    GeneralPos PosZero, Pos1, Pos1a, Pos2, Pos2a, Pos3, Pos3a, Pos4, Pos4a;

    Hsc3::Comm::CommApi cmApi("");
    Hsc3::Proxy::ProxyMotion pMot(&cmApi);

    loadPosData(PosZero, 25, -55, 118, 58, true); // 机械臂总原点

    if (connectIPC(cmApi,"10.10.56.214",23234)) {
        pMot.setOpMode(OP_T1);
        pMot.setWorkFrame(0, FRAME_BASE); // FRAME_BASE: 用户坐标系
        pMot.setWorkpieceNum(0, 0); // setWorkpieceNum(int8_t gpId, int8_t num): gpId 组号，num 工件坐标索引(0..15)

        //机器人上使能
        pMot.setGpEn(0, true);
        sleep(1);
        pMot.moveTo(0, PosZero, false); // true: 直线运动, false: 关节运动
        waitDone(pMot);
        std::cout << "已运动到关节坐标系设置的原点" << std::endl;

        sleep(3); // 等待yolo进行识别

        // 以下是套杯流程
        // ------------------------- 1号乳头套杯 -------------------------
        while (1) {
            if (countLines(file_txt) == 4) {
                std::cout << "此次识别到了四个乳头" << std::endl;
                // 读取第一行坐标
                readPos(pre_coordinate, file_txt, 1);
                break;
            } else {
                std::cout << "此次未识别到四个乳头" << std::endl;
            }
        }
        loadPosData(Pos1, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
        loadPosData(Pos1a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);
        
        pMot.moveTo(0, Pos1, true);
		waitDone(pMot);
        // 末端执行器逻辑
        pMot.moveTo(0, Pos1a, true);
		waitDone(pMot);
        // 末端执行器逻辑

        pMot.moveTo(0, PosZero, true);
		waitDone(pMot);

        if (countLines(file_txt) > 3) {
            std::cout << "1号乳头第一次套杯失败，进行第二次" << std::endl;
            // 重新读取坐标
            readPos(pre_coordinate, file_txt, 1);
            loadPosData(Pos1, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
            loadPosData(Pos1a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

            pMot.moveTo(0, Pos1, true);
            waitDone(pMot);
            // 末端执行器逻辑
            pMot.moveTo(0, Pos1a, true);
            waitDone(pMot);
            // 末端执行器逻辑

            pMot.moveTo(0, PosZero, true);
            waitDone(pMot);
        }

        if (countLines(file_txt) == 3) {
            std::cout << "1号乳头套杯成功" << std::endl;
        } else {
            std::cout << "1号乳头套杯失败，进行2号乳头套杯" << std::endl;
        }

        // ------------------------- 2号乳头套杯 -------------------------
        sleep(1);
        while (1) {
            if (countLines(file_txt) == 3) {
                std::cout << "此次识别到了三个乳头" << std::endl;
                // 识别到了3个乳头(第一次套杯成功，1号乳头识别不到)，读取第一个乳头的坐标(第一个指文件第一行)
                readPos(pre_coordinate, file_txt, 1);
                break;
            } else if (countLines(file_txt) == 4) {
                std::cout << "此次识别到了四个乳头" << std::endl;
                // 识别到了4个乳头(第一次套杯失败，仍能识别到1号乳头)，此时读取2号乳头坐标，取文件第二行
                readPos(pre_coordinate, file_txt, 2);
                break;
            } else {
                std::cout << "此次未识别到三个乳头" << std::endl;
            }
        }

        loadPosData(Pos2, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
        loadPosData(Pos2a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

        pMot.moveTo(0, Pos2, true);
		waitDone(pMot);
        // 末端执行器逻辑
        pMot.moveTo(0, Pos2a, true);
		waitDone(pMot);
        // 末端执行器逻辑

        pMot.moveTo(0, PosZero, true);
		waitDone(pMot);
        
        if (countLines(file_txt) > 2) {
            std::cout << "2号乳头第一次套杯失败，进行第二次" << std::endl;
            // 重新读取坐标
            if (countLines(file_txt) == 3) {
                readPos(pre_coordinate, file_txt, 1);
            } else if (countLines(file_txt) == 4) {
                readPos(pre_coordinate, file_txt, 2);
            }
            loadPosData(Pos2, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
            loadPosData(Pos2a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

            pMot.moveTo(0, Pos2, true);
            waitDone(pMot);
            // 末端执行器逻辑
            pMot.moveTo(0, Pos2a, true);
            waitDone(pMot);
            // 末端执行器逻辑

            pMot.moveTo(0, PosZero, true);
            waitDone(pMot);
        }

        if (countLines(file_txt) == 2) {
            std::cout << "2号乳头套杯成功" << std::endl;
        } else {
            std::cout << "2号乳头套杯失败，进行3号乳头套杯" << std::endl;
        }

        // ------------------------- 3号乳头套杯 -------------------------
        sleep(1);
        while (1) {
            if (countLines(file_txt) == 2) {
                std::cout << "此次识别到了两个乳头" << std::endl;
                // 识别到2个乳头，说明第一次和第二次套杯都成功了，此时直接读取文件第1行数据就是3号乳头的坐标
                readPos(pre_coordinate, file_txt, 1);
                break;
            } else if (countLines(file_txt) == 3) {
                std::cout << "此次识别到了三个乳头" << std::endl;
                // 识别到3个乳头，说明第一次或者第二次套杯有一次失败，此时读取文件第2行数据就是3号乳头的坐标
                readPos(pre_coordinate, file_txt, 2);
                break;
            } else if (countLines(file_txt) == 4) {
                std::cout << "此次识别到了四个乳头" << std::endl;
                // 识别到4个乳头，说明第一次和第二次套杯都失败了，此时读取文件第3行数据，即3号乳头的坐标
                readPos(pre_coordinate, file_txt, 3);
                break;
            } else {
                std::cout << "此次未识别到两个乳头" << std::endl;
            } 
        }

        loadPosData(Pos3, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
        loadPosData(Pos3a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

        pMot.moveTo(0, Pos3, true);
		waitDone(pMot);
        // 末端执行器逻辑
        pMot.moveTo(0, Pos3a, true);
		waitDone(pMot);
        // 末端执行器逻辑

        pMot.moveTo(0, PosZero, true);
		waitDone(pMot);

        if (countLines(file_txt) > 1) {
            std::cout << "3号乳头第一次套杯失败，进行第二次" << std::endl;
            // 重新读取坐标
            if (countLines(file_txt) == 2) {
                readPos(pre_coordinate, file_txt, 1);
            } else if (countLines(file_txt) == 3) {
                readPos(pre_coordinate, file_txt, 2);
            } else if (countLines(file_txt) == 4) {
                readPos(pre_coordinate, file_txt, 3);
            }
            loadPosData(Pos3, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
            loadPosData(Pos3a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

            pMot.moveTo(0, Pos3, true);
            waitDone(pMot);
            // 末端执行器逻辑
            pMot.moveTo(0, Pos3a, true);
            waitDone(pMot);
            // 末端执行器逻辑

            pMot.moveTo(0, PosZero, true);
            waitDone(pMot);
        }

        if (countLines(file_txt) == 1) {
            std::cout << "3号乳头套杯成功" << std::endl;
        } else {
            std::cout << "3乳头套杯失败，进行4号乳头套杯" << std::endl;
        }

        // ------------------------- 4号乳头套杯 -------------------------
        sleep(1);
        while (1) {
            if (countLines(file_txt) == 1) {
                std::cout << "此次识别到了一个乳头" << std::endl;
                // 识别到1个乳头，说明前三次套杯都成功了，此时直接读取文件第1行数据就是4号乳头的坐标
                readPos(pre_coordinate, file_txt, 1);
                break;
            } else if (countLines(file_txt) == 2) {
                std::cout << "此次识别到了两个乳头" << std::endl;
                // 识别到2个乳头，说明前三次套杯其中有一次失败，此时读取文件第2行数据就是4号乳头的坐标
                readPos(pre_coordinate, file_txt, 2);
                break;
            } else if (countLines(file_txt) == 3) {
                std::cout << "此次识别到了三个乳头" << std::endl;
                // 识别到3个乳头，说明前三次套杯其中有两次失败，此时读取文件第3行数据就是4号乳头的坐标
                readPos(pre_coordinate, file_txt, 3);
                break;
            } else if (countLines(file_txt) == 4) {
                std::cout << "此次识别到了四个乳头" << std::endl;
                // 识别到4个乳头，说明前四次套杯全部失败，此时读取文件第4行数据就是4号乳头的坐标
                readPos(pre_coordinate, file_txt, 4);
                break;
            } else {
                std::cout << "此次未识别到一个乳头" << std::endl;
            } 
        }

        loadPosData(Pos4, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
        loadPosData(Pos4a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

        pMot.moveTo(0, Pos4, true);
		waitDone(pMot);
        // 末端执行器逻辑
        pMot.moveTo(0, Pos4a, true);
		waitDone(pMot);
        // 末端执行器逻辑

        pMot.moveTo(0, PosZero, true);
		waitDone(pMot);

        if (countLines(file_txt) > 0) {
            std::cout << "4号乳头第一次套杯失败，进行第二次" << std::endl;
            // 重新读取坐标
            if (countLines(file_txt) == 1) {
                readPos(pre_coordinate, file_txt, 1);
            } else if (countLines(file_txt) == 2) {
                readPos(pre_coordinate, file_txt, 2);
            } else if (countLines(file_txt) == 3) {
                readPos(pre_coordinate, file_txt, 3);
            } else if (countLines(file_txt) == 4) {
                readPos(pre_coordinate, file_txt, 4);
            }
            loadPosData(Pos4, pre_coordinate[0], pre_coordinate[1], -10, -10, false);
            loadPosData(Pos4a, pre_coordinate[0], pre_coordinate[1], pre_coordinate[2], -10, false);

            pMot.moveTo(0, Pos4, true);
            waitDone(pMot);
            // 末端执行器逻辑
            pMot.moveTo(0, Pos4a, true);
            waitDone(pMot);
            // 末端执行器逻辑

            pMot.moveTo(0, PosZero, true);
            waitDone(pMot);
        }

        if (countLines(file_txt) == 0) {
            std::cout << "4号乳头套杯成功" << std::endl;
        } else {
            std::cout << "4乳头套杯失败" << std::endl;
        }

        std::cout << "套杯动作完成" << std::endl;
    } else {
        std::cout << "连接失败" << std::endl;
    }

    //机器人下使能
    pMot.setGpEn(0, false);
    //断连
    disconnectIPC(cmApi);

    system("pause");
    return 0;
}

/************************************************************************/
/*            
* @brief 连接IPC
* @param cmApi:通信客户端对象，建议在自定义的函数中如果需要传递客户端对象，都使用引用传递。
* @param strIP:IP，控制器默认IP是"10.10.56.214"
* @param uPort:端口号,固定端口号:23234
*/
/************************************************************************/
bool connectIPC(Hsc3::Comm::CommApi& cmApi, std::string strIP,uint16_t uPort) {
    //1.设置非自动重连模式,连接前调用。
    cmApi.setAutoConn(false);
    //2.连接
    Hsc3::Comm::HMCErrCode ret = cmApi.connect(strIP,uPort);
    if (ret!=0) {
        printf("CommApi::connect() : ret = %lld\n",ret);
    }
    //3.查询是否连接
    if (cmApi.isConnected()) {
        std::cout << "连接成功" << std::endl;
        return true;
    } else {
        std::cout << "连接失败" << std::endl;
        return false;
    }
}

/************************************************************************/
/* 
* @brief 断开与IPC的连接
* @param cmApi:通信客户端对象
*/
/************************************************************************/
bool disconnectIPC(Hsc3::Comm::CommApi& cmApi) {
    Hsc3::Comm::HMCErrCode ret=cmApi.disconnect();
    sleep(5);
    if (cmApi.isConnected()) {
        return false;
    } else {
        return true;
    }
}

/************************************************************************/
/* 
 * @brief 装载数据
 * @param isJoint: 是否关节点，和moveTo()函数的第3个参数(isLinear: 是否直线运动、关节运动)有关
 *                 isJoint是true，则isLinear为false(是关节点，则关节运动)；isJoint是false，则isJoint为true(不是关节点，则直线运动)
 */
/************************************************************************/
void loadPosData(GeneralPos& generalPos, double axis0, double axis1, double axis2, double axis3, bool isJoint) {
	generalPos.config = 1048576;
	generalPos.isJoint = isJoint; // 是否关节点
	generalPos.ufNum = 0; // 工件号(修改为对应的用户坐标系的工件坐标)
	generalPos.utNum = -1; // 工具号
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
void waitDone(Hsc3::Proxy::ProxyMotion& pMot) {
    ManState manualState = MAN_STATE_MAX;
    //此延时是为了防止机器人还未进入运动状态
    sleep(1);

    while(1) {
        pMot.getManualStat(manualState);
        if (manualState == MAN_STATE_WAIT || manualState == MAN_STATE_ERROR) {
            if (manualState == MAN_STATE_ERROR) {
                std::cout << "错误" << std::endl;
            }
            break;
        }
    }
}

int countLines(fstream& file) {
    file.open(Binocular_camera_path, ios::in);
    
    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        count++;
    }

	file.close();
    return count;
}

void readPos(vector<double>& pre_coordinate, fstream& file, int index) {
    pre_coordinate.clear();
    file.open(Binocular_camera_path, ios::in);
    string str;

    while (index--) {
        getline(file, str);
    }

    int j = 0;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == ' ') {
            pre_coordinate.push_back(stod(str.substr(j, i - j)));
            j = i + 1;
        }
    }
    file.close();
}
