#include <QDebug>

#include "fileparser.h"

/* This function check parameters */
QFileInfoList GetFileList(QString &path)
{
    QFileInfoList filelist;

    if(path.isEmpty())
        return filelist;

    QFileInfo fi(path);
    if(!fi.exists())
        return filelist;

    if(fi.isDir())
        filelist.append(GetDirFileList(path));

    if(fi.isFile())
        filelist.push_back(fi);

    return filelist;
}

/* This function does not check parameters */
QFileInfoList GetDirFileList(QString &path)
{
    QDir dir(path);
    QFileInfoList filelist = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folderlist = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for(int i = 0; i != folderlist.size(); i++)
    {
         QString name = folderlist.at(i).absoluteFilePath();
         QFileInfoList subfilelist = GetFileList(name);
         filelist.append(subfilelist);
    }

    return filelist;
}

ByteArrayList GetMultiFileData(QFileInfoList &flist)
{
    ByteArrayList pktlist;

    if(flist.isEmpty())
        return pktlist;

    foreach (QFileInfo finfo, flist) {
        QString filename = finfo.absoluteFilePath();
        ByteArrayList readpkt = GetFileData(filename);
        pktlist.insert(pktlist.end(), readpkt.begin(), readpkt.end());
    }

    return pktlist;
}

/* This function does not check parameters */
ByteArrayList GetFileData(QString &filename)
{
    ByteArrayList pktlist;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << QStringLiteral("Fail to open file %1.").arg(filename) << Qt::endl;
        return pktlist;
    }

    ByteArray packet;
    while(!file.atEnd())
    {
        QByteArray oneline = file.readLine();
        oneline = QByteArray::fromHex(oneline);
        packet.assign(oneline.begin(), oneline.end());
        pktlist.push_back(packet);
    }

    file.close();
    return pktlist;
}

void SaveDataToFile(QString &data, QString &filename)
{
    QString output;
    if(!filename.isEmpty())
    {
        QFile file(filename);
        if(!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
        {
            qDebug() << QStringLiteral("Fail to open file %1.").arg(filename) << Qt::endl;
            return;
        }

        QTextStream fileOut(&file);

        fileOut << data << Qt::endl;
        file.flush();
        file.close();
    }

    return;
}
