#ifndef FILEPARSER_H
#define FILEPARSER_H


#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

#include "fuzzerutil.h"

QFileInfoList GetFileList(QString &path);
QFileInfoList GetDirFileList(QString &path);
ByteArrayList GetFileData(QString &file_name);
ByteArrayList GetMultiFileData(QFileInfoList &flist);
void SaveDataToFile(QString &data, QString &filename);

#endif // FILEPARSER_H