﻿/**
*   Copyright (C) 2018 华数机器人
*
*   @file       ProxyExt.h
*   @brief      华数III型二次开发接口 - 业务接口 - 外部操作代理
*   @details    提供了III型控制器外部模式相关业务接口。
*
*   @author
*   @date       2018/10/16
*   @version
*
*/

#pragma once
/**
*   @skip DLL_EXPORT
*/
#ifdef _LINUX_
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT _declspec(dllexport)
#endif

#include "MdlExtDef.h"
#include "CommDef.h"
#include <stdint.h>
#include <list>

namespace Hsc3 {
    namespace Comm {
        class CommApi;
    }

    namespace Proxy {
        /**
        *   @class      ProxyExt
        *   @brief      业务接口 - 外部操作代理
        *   @details    提供接口包含：外部操作接口。
        *   @date       2018/10/16
        */
        class DLL_EXPORT ProxyExt
        {
        public:
            /**
             * @brief   构造函数
             * @details 注：确保传入已构造的通信客户端。
             * @param   pNet    通信客户端
             */
            ProxyExt(Hsc3::Comm::CommApi * pNet);

            ~ProxyExt();

	        /**
             * @brief   获取外部输入配置数据
             * @details
             * @param[out]   extInData   输入信号配置信息列表
             * @see     addExtIn delExtIn
             */
	        Hsc3::Comm::HMCErrCode getExtIn(std::list<Hsc3::Ext::HookExtIn> & extInData);

	        /**
             * @brief   添加外部输入配置
             * @details
             * @param   extInItem   待添加项
             * @see     getExtIn delExtIn
             */
	        Hsc3::Comm::HMCErrCode addExtIn(const Hsc3::Ext::HookExtIn & extInItem);

	        /**
             * @brief   删除外部输入配置
             * @details
             * @param   extInItem   待删除项
             * @see     getExtIn addExtIn
             */
	        Hsc3::Comm::HMCErrCode delExtIn(const Hsc3::Ext::HookExtIn & extInItem);

	        /**
             * @brief   获取外部输出配置数据
             * @details
             * @param[out]   extOutData   输出信号配置信息列表
             * @see     addExtOut delExtOut
             */
	        Hsc3::Comm::HMCErrCode getExtOut(std::list<Hsc3::Ext::HookExtOut> & extOutData);

	        /**
             * @brief   添加外部输出配置
             * @details
             * @param   extOutItem   待添加项
             * @see     getExtOut delExtOut
             */
	        Hsc3::Comm::HMCErrCode addExtOut(const Hsc3::Ext::HookExtOut & extOutItem);

	        /**
             * @brief   删除外部输出配置
             * @details
             * @param   extOutItem   待删除项
             * @see     getExtOut addExtOut
             */
	        Hsc3::Comm::HMCErrCode delExtOut(const Hsc3::Ext::HookExtOut & extOutItem);

	        /**
             * @brief   获取外部程序
             * @details 可由外部信号控制的程序
             * @param[out]   progName   程序名
             * @see     setExtProgName
             */
	        Hsc3::Comm::HMCErrCode getExtProgName(std::string & progName);

	        /**
             * @brief   设置外部程序
             * @details 可由外部信号控制的程序
             * @param   progName   程序名
             * @see     getExtProgName
             */
	        Hsc3::Comm::HMCErrCode setExtProgName(const std::string & progName);

			 /**
			 * @version 1.6以上
             * @brief   设置外部运行程序
             * @details 可由外部信号控制的程序
			 * @param   progName   程序名
			 * @param   index	   配置项编号
             * @see     setExtProgName
             */
	        Hsc3::Comm::HMCErrCode setExtProgName( std::string & progName, int index);

			/**
			 * @version 1.6以上
             * @brief   获取外部运行程序映射表
             * @details 可由外部信号控制的程序
			 * @param[out]   progNameMap   程序名映射表（格式：中括号扩起、半角逗号分隔的外部运行程序配置映射表，例“[ 1=X.PRG, 2=/a/Y.PRG ]”）
             * @see     getExtProgNameMap
             */
	        Hsc3::Comm::HMCErrCode getExtProgNameMap(std::string & progNameMap);

			/**
			 * @version 1.6以上
             * @brief   设置外部运行程序编号绑定寄存器
             * @details 可由外部信号控制的程序
			 * @param   iR   R寄存器索引（0..n-1）
             * @see     setR4ExtProg
             */
	        Hsc3::Comm::HMCErrCode setR4ExtProg(int iR);

			/**
			 * @version 1.6以上
             * @brief   获取外部运行程序编号绑定寄存器
             * @details 可由外部信号控制的程序
			 * @param[out]   iR   R寄存器索引（0..n-1）
             * @see     setR4ExtProg
             */
	        Hsc3::Comm::HMCErrCode getR4ExtProg(int32_t & iR);

	        /**
             * @brief   获取引用寄存器数据
             * @details
             * @param[out]   refRegData   数据列表
             * @see     modifyRefRegData delRefRegData
             */
	        Hsc3::Comm::HMCErrCode getRefRegData(std::list<Hsc3::Ext::RefRegData> & refRegData);

	        /**
             * @brief   修改（添加）引用寄存器数据
             * @details
             * @param   modData   待修改的数据
             * @see     getRefRegData delRefRegData
             */
	        Hsc3::Comm::HMCErrCode modifyRefRegData(const Hsc3::Ext::RefRegModData & modData);

	        /**
             * @brief   删除引用寄存器数据
             * @details
             * @param   iRef   引用信号索引
             * @see     getRefRegData modifyRefRegData
             */
	        Hsc3::Comm::HMCErrCode delRefRegData(int iRef);

	        /**
             * @brief   获取编码列表
             * @details
             * @param[out]   enCoderData   数据列表
             * @see
             */
	        Hsc3::Comm::HMCErrCode getEnCoderData(std::list<Hsc3::Ext::EnCoderData> & enCoderData);

			/**
             * @brief   添加编码数据
             * @details
             * @param[out]   enCoderData   数据
             * @see
             */
	        Hsc3::Comm::HMCErrCode addEncoder(const Hsc3::Ext::EnCoderData & enCoderData);

			/**
             * @brief   删除编码数据
             * @details
             * @param[out]   enCoderData   数据
             * @see
             */
	        Hsc3::Comm::HMCErrCode delEncoder(const Hsc3::Ext::EnCoderData & enCoderData);

			 /**
             * @brief   获取解码列表
             * @details
             * @param[out]   enCoderData   数据列表
             * @see
             */
	        Hsc3::Comm::HMCErrCode getDeCoderData(std::list<Hsc3::Ext::DeCoderData> & deCoderData);

			/**
             * @brief   添加解码数据
             * @details
             * @param[out]   enCoderData   数据
             * @see
             */
	        Hsc3::Comm::HMCErrCode addDecoder(const Hsc3::Ext::DeCoderData & deCoderData);

			/**
             * @brief   删除解码数据
             * @details
             * @param[out]   enCoderData   数据
             * @see
             */
	        Hsc3::Comm::HMCErrCode delDecoder(const Hsc3::Ext::DeCoderData & deCoderData);

            /**
             * @brief   把当前IO配置写入到文件
             * @param
             */
	        Hsc3::Comm::HMCErrCode save();

        private:

	        inline Hsc3::Comm::HMCErrCode execContackOnHook(const std::string & strIntent, const std::string & strMsg, std::string & strResponse);

        private:
            Hsc3::Comm::CommApi * m_pNet;
        };
	}
}