/*
 * getpath.h
 *
 *  Created on: Jul 23, 2020
 *      Author: jerry
 */

#ifndef SRC_GETPATH_H_
#define SRC_GETPATH_H_

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
char* getFilePathByLink(const char *filename);

//通过链接文件名获取被目标文件所在目录
char* getFileDirByLink(const char *linkfile);

#endif /* SRC_GETPATH_H_ */
