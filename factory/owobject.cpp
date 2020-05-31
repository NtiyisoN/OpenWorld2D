/*!
     Copyright 2020. Coding Studio. All rights reserved.
     BSD License Usage

     [Coding Studio]
     Git     : https://github.com/devghframework/OpenWorld2D
     email   : devlee.freebsd@gmail.com
     twitch  : https://www.twitch.tv/codingstudio
     youtube : https://www.youtube.com/channel/UCMj3LpAxKiBmPeScDkan0sg?view_as=subscriber
 */

#include "factory/owobject.h"

#include <QDebug>
#include <QImageReader>
#include <QMap>
#include <QMessageBox>
#include <QPainter>
#include <QString>
#include <QThread>

OwObject::OwObject(Isometric *isometric) : IOwObject()
{
    this->m_isometric = isometric;
    this->m_objectStatus = IOwObject::STATUS_NONE;

    connect(&m_actionTimer, SIGNAL(timeout()), this, SLOT(ObjectAction()));
}

OwObject::~OwObject()
{

}

void OwObject::ObjectAction()
{
    if (this->m_moveEndPointPixel.x() > this->m_moveStartPointPixel.x()) {
        this->m_moveStartPointPixel.setX(
            this->m_moveStartPointPixel.x()
            + this->m_splitObjectInfo[IOwObject::DIRECTION_S]->movePixel);
    } else if (this->m_moveEndPointPixel.x() < this->m_moveStartPointPixel.x()) {
        this->m_moveStartPointPixel.setX(
            this->m_moveStartPointPixel.x()
            - this->m_splitObjectInfo[IOwObject::DIRECTION_S]->movePixel);
    }

    if (this->m_moveEndPointPixel.y() > this->m_moveStartPointPixel.y()) {
        this->m_moveStartPointPixel.setY(
            this->m_moveStartPointPixel.y()
            + this->m_splitObjectInfo[IOwObject::DIRECTION_S]->movePixel);
    } else if (this->m_moveEndPointPixel.y() < this->m_moveStartPointPixel.y()) {
        this->m_moveStartPointPixel.setY(
            this->m_moveStartPointPixel.y()
            - this->m_splitObjectInfo[IOwObject::DIRECTION_S]->movePixel);
    }

    if (this->m_moveStartPointPixel.x() == this->m_moveEndPointPixel.x()
        && this->m_moveStartPointPixel.y() == this->m_moveEndPointPixel.y()) {
        qDebug(">>>> 도착함.....");

        this->m_destination = IOwObject::DESTINATION_ARRIVED;
        ArrivedDestinationEvent();

        this->m_objectStatus = IOwObject::STATUS_NONE;
        this->m_actionTimer.stop();
        this->m_animationNo = 0;
    }
}

int OwObject::CheckDirection(int moveOldX, int moveOldY, int moveX, int moveY)
{
    enum Dir {
        N = 0b00000001, // 1
        S = 0b00000010, // 2
        E = 0b00000100, // 4
        W = 0b00001000, // 8
    };

    int direction = Dir::S;

    if (moveOldY > moveY) {
        direction = Dir::S;
    } else if (moveOldY < moveY) {
        direction = Dir::N;
    }
    if (moveOldX > moveX) {
        direction |= Dir::E;
    } else if (moveOldX < moveX) {
        direction |= Dir::W;
    }
    return direction;
}

void OwObject::CreateObject() {}

void OwObject::DrawObject(QPainter *painter)
{
    if (this->m_objectStatus == IOwObject::STATUS_NONE) {
        this->m_animationNo = 0;
    }
    if (this->m_splitObjectInfo.count() > 0) {
        painter->drawPixmap(this->m_moveStartPointPixel.x(),
                            this->m_moveStartPointPixel.y(),
                            this->m_splitObjectInfo[IOwObject::DIRECTION_S]
                                ->splitImage[this->m_animationNo]);
        this->m_animationNo++;
        if (this->m_animationNo
            >= this->m_splitObjectInfo[IOwObject::DIRECTION_S]->totalSplitCount) {
            this->m_animationNo = 0;
        }
    }
}

void OwObject::MouseDown(int metricX, int metricY)
{
    if (this->m_objectStatus == IOwObject::STATUS_NONE) {
        //        this->m_moveStartPoint.setX(metricX);
        //        this->m_moveStartPoint.setY(metricY);
        //        this->m_moveStartPointPixel = this->m_isometric->GetMetricPixel(metricX, metricY);
        this->m_objectStatus = IOwObject::STATUS_WORK;

    } else if (this->m_objectStatus == IOwObject::STATUS_WORK) {
        this->m_moveEndPoint.setX(metricX);
        this->m_moveEndPoint.setY(metricY);
        this->m_moveEndPointPixel = this->m_isometric->GetMetricPixel(metricX, metricY);

        this->m_animationNo = 0;
        this->m_destination = IOwObject::DESTINATION_START;

        this->m_actionTimer.start(50);
    }
}

void OwObject::MouseUp(int metricX, int metricY)
{
    Q_UNUSED(metricX);
    Q_UNUSED(metricY);
}

void OwObject::MouseMove(int metricX, int metricY)
{
    Q_UNUSED(metricX);
    Q_UNUSED(metricY);
}

void OwObject::SetObjectSplitImageInfo(QMap<int, ObjectSplitImageInfo *> objInfo)
{
    this->m_splitObjectInfo = objInfo;
    CreateSplitImage();
}

//QMap<int, ObjectSplitImageInfo *> OwObject::GetObjectSplitImageInfo()
//{
//    return this->m_splitObjectInfo;
//}

void OwObject::CreateSplitImage()
{
    for (auto key : this->m_splitObjectInfo.keys()) {
        QString fileName = this->m_splitObjectInfo.value(key)->originFileName;
        QImageReader r(fileName);

        QImage i = r.read();
        if (!i.isNull()) {
            QPixmap pixmap;
            pixmap = QPixmap::fromImage(i);

            this->m_splitObjectInfo.value(key)->splitImage
                = new QPixmap[this->m_splitObjectInfo.value(key)->totalSplitCount];

            switch (this->m_splitObjectInfo.value(key)->copyDirection) {
            case IOwObject::COPY_RIGHT: {
                int x = this->m_splitObjectInfo.value(key)->copyStartPoint.x();
                int y = this->m_splitObjectInfo.value(key)->copyStartPoint.y();
                for (int idx = 0; idx < this->m_splitObjectInfo.value(key)->totalSplitCount; idx++) {
                    this->m_splitObjectInfo.value(key)->splitImage[idx]
                        = pixmap.copy(x,
                                      y,
                                      this->m_splitObjectInfo.value(key)->width,
                                      this->m_splitObjectInfo.value(key)->height);
                    x += this->m_splitObjectInfo.value(key)->width;
                }
                break;
            }
            case IOwObject::COPY_BOTTOM: {
                int x = this->m_splitObjectInfo.value(key)->copyStartPoint.x();
                int y = this->m_splitObjectInfo.value(key)->copyStartPoint.y();
                for (int idx = 0; idx < this->m_splitObjectInfo.value(key)->totalSplitCount; idx++) {
                    this->m_splitObjectInfo.value(key)->splitImage[idx]
                        = pixmap.copy(x,
                                      y,
                                      this->m_splitObjectInfo.value(key)->width,
                                      this->m_splitObjectInfo.value(key)->height);
                    y += this->m_splitObjectInfo.value(key)->height;
                }
                break;
            }
            default: {
                QMessageBox msgBox;
                msgBox.setText("복사할 방향을 지정하지 않았습니다.");
                msgBox.setInformativeText(
                    "원본이미지에서 복사할 방향을 지정해 주십시오.\r\n(RIGHT 또는 BOTTOM)");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();
                break;
            }
            }
        } else {
            QMessageBox msgBox;
            msgBox.setText("이미지 파일을 읽을 수 없습니다.");
            msgBox.setInformativeText("이미지 파일 위치와 이미지 포멧을 확인해 주십시오.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        }
    }
}

void OwObject::ArrivedDestinationEvent()
{
    if (this->m_destination == IOwObject::DESTINATION_ARRIVED) {
        // TODO : 도착처리를 한다.

        // 도착지를 초기화한다.
        this->m_destination = IOwObject::NO_DESTINATION;
    }
}
