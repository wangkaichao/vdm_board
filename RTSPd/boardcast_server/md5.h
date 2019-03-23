/****************************************************************************
**
** Copyright (C) 2010 好视网络科技有限公司.
** All rights reserved.
**
** author:  liz
** mail:    liz@gvtv.com.cn / QQ: / MSN:
** date:    2012-08-22
** des:     md5
**
****************************************************************************/
#ifndef MD5_H
#define MD5_H



#ifdef __cplusplus
extern "C" {
#endif

/*!
* 计算md5值
* @param[in] filepath 文件路径
* @param[out] data MD5计算后的结果16位
* @return 无
*/
int create_md5_value_by_file_16(unsigned char *filepath,unsigned char *data);

#ifdef __cplusplus
}
#endif

#endif // MD5_H
