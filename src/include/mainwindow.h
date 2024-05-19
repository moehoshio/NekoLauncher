#pragma once

#include "network.h"


#include <memory>
#include <string>


#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QObject>
#include <QtCore/QMetaObject>


#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>


#include <QtGui/QScreen>


#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtGui/QGuiApplication>


#include <SimpleIni/SimpleIni.h>

namespace ui{


class MainWindow : public QMainWindow{
    Q_OBJECT
private:
    QWidget *widget = new QWidget;
    QLabel *backgroundLabel;
    QIcon *icon;
    QGridLayout *gridLayout;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
    const QSize scrSize = QGuiApplication::primaryScreen()->size();

public: MainWindow(QWidget *parent = nullptr) : QMainWindow(parent){
        setBase();
        defaultConstructor();
    }

    MainWindow(const CSimpleIniA &ini, QWidget *parent = nullptr) : QMainWindow(parent){

        nlog::Info(FI, LI, "%s : config Constructor  , obj empty : %s", FN, exec::boolTo<const char *>(ini.IsEmpty()));
        setBase();

        if (!ini.IsEmpty())
            configInitBg(ini);
        else
            defaultConstructor();
    }

    void setBase();
    void defaultConstructor();
    void configInitBg(const CSimpleIniA &ini);

    void setImgbgBase();
    void setVidbgBase();

    void setImgbg(const char *file);
    void setImgbg(std::shared_ptr<QByteArray> &&data);
    void setVidbg(const std::string &file);
    void setVidbg(QUrl url);


    ~MainWindow() {
        delete backgroundLabel;
        delete icon;
        delete gridLayout;
        delete player;
        delete videoWidget;
    };

void onVidDgCallback(std::string filePath);

void onImgDgCallback(std::shared_ptr<QByteArray> data);

signals:
    void resourceVideo(std::string filePath);
    void resourceImg(std::shared_ptr<QByteArray> data);

};
} // namespace ui
#include "mainwindow.moc"