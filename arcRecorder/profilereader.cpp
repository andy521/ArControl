
#include "profilereader.h"
#include "globalparas.h"
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

using namespace PROFILEREADER_PARA;
using namespace PROFILEREADER_PRIVATE;
using namespace GLOBAL_PARA;

ProfileReader * ProfileReader::instance = NULL;
ProfileReader::ProfileReader(QObject *parent):QObject(parent)
{
}
ProfileReader::ProfileReader(const ProfileReader&):QObject(nullptr)
{
}
ProfileReader& ProfileReader::operator=(const ProfileReader&)
{
}

ProfileReader* ProfileReader::getInstance()
{
    if(ProfileReader::instance == NULL){
        ProfileReader::instance = new ProfileReader();
        ProfileReader::instance->checkProfile();
    }
    return ProfileReader::instance;
}

QString ProfileReader::getArduino()
{
    if(!this->hasChecked)
        this->checkProfile();
    this->hasChecked=true;
    return this->arduino_debug;
}

void ProfileReader::checkProfile(bool newapath)
{
    QFileInfo finfo(PROFILE_FILE);
    QFile f(PROFILE_FILE);
    QDomDocument doc("doc");
    if(!finfo.exists() || !finfo.isFile()){
        /* create */
        qDebug()<<"No profile exsited, will create!";
        Q_ASSERT(f.open(QFile::ReadWrite | QFile::Text));
        doc.setContent(&f);
        QDomProcessingInstruction  instruction =  doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        QTextStream out(&f);
        doc.save(out, QDomNode::EncodingFromDocument);
        f.close();
    }
    /* 矫正 文件 */
    qDebug()<<"Profile exsited";
    SCPP_ASSERT_THROW(f.open(QFile::ReadWrite | QFile::Text), "cannot open");
    doc.clear();
    doc.setContent(&f);
    QDomElement root = doc.documentElement();
    if(root.isNull()){
        root = doc.createElement(DOM_PROFILE);
        doc.appendChild(root);
    }
    QDomElement dom_arduino = root.firstChildElement(DOM_ARDUINO);
    if(dom_arduino.isNull()){
        dom_arduino = doc.createElement(DOM_ARDUINO);
        root.appendChild(dom_arduino);
    }
    if(dom_arduino.firstChildElement().isNull()){
        dom_arduino.appendChild(doc.createTextNode(""));
    }
    QString arduino_path = dom_arduino.text();

    /* 判断平台 */
    QString debugFile;
    bool reChoose = false;
    QFileInfo debuger(arduino_path);
#ifdef Q_OS_WIN
    debugFile = "arduino_debug.exe";
#endif
#ifdef Q_OS_LINUX
    debugFile = "arduino";
#endif
#ifdef Q_OS_MAC
    debugFile = "arduino.app";
#endif
    reChoose =(debugFile.isEmpty() || !debuger.exists() || !debuger.isFile() || !debuger.isExecutable());
    if(newapath || reChoose){
        QMessageBox::about(0, tr("About"),
                           tr("Choose Arduino IDE according to OS:\n"\
                              "-- WINDOWS: arduino_path/arduino_debug.exe\n"\
                              "-- LINUX  : arduino_path/arduino\n"\
                              "-- MAC    : arduino_path/arduino.app\n\n" \
                              "<Download>: https://www.arduino.cc/en/Main/Software\n" \
                              "<  Note  >: the Arduino IDE edition should be greater than 1.6.8!\n"\
                              "<  Note  >: You'd better set it within ARCDESIGNER>Profile!"));
         arduino_path = QFileDialog::getOpenFileName(0, tr("Choose Arduino IDE"),
                                                     "", debugFile); //may rais an-warning on debug-output, don't warry
         if(!arduino_path.isEmpty())
            dom_arduino.firstChild().setNodeValue(arduino_path);
    }
    f.resize(0);
    QTextStream out(&f);
    doc.save(out, QDomNode::EncodingFromDocument);
    f.close();
    this->arduino_debug = dom_arduino.text();
}

void ProfileReader::reArduinoPath()
{
    ProfileReader::checkProfile(true);
}
