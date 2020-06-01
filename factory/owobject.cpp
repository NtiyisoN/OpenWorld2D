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
#include <openworldwidget/owdrawwidget.h>

#include <QDebug>
#include <QImageReader>
#include <QMap>
#include <QMessageBox>
#include <QPainter>
#include <QString>
#include <QThread>

/*!
 * \brief OwObject::OwObject 객체 생성 함수
 * \param isometric
 */
OwObject::OwObject(Isometric *isometric) : IOwObject()
{
    this->m_isometric = isometric;
    this->m_objectStatus = IOwObject::STATUS_NONE;
    this->m_actionName = IOwObject::GetMetaEnum(IOwObject::OBJECT_MOVE_DIRECTION::DIRECTION_S);

    connect(&m_actionTimer, SIGNAL(timeout()), this, SLOT(ObjectAction()));
}

/*!
 * \brief OwObject::~OwObject 객체 파괴 함수
 */
OwObject::~OwObject()
{

}

/*!
 * \brief OwObject::ObjectAction 오브젝트의 움직임을 계산 하는 함수
 * TODO : 현재 움직임 단위가 픽셀인데, 매트릭 단위로 움직이도록 변경해야 한다.
 */
void OwObject::ObjectAction()
{
    // 목적지에 도착하였을 경우 움직임 처리
    if (this->m_objectStatus == IOwObject::STATUS_ARRIVED) {
        if (this->m_animationNo
            >= (this->m_splitObjectInfo[this->m_actionName]->totalSplitCount - 1)) {
            this->m_objectStatus = IOwObject::STATUS_DONE;
            return;
        }
    }

    // 모든 처리가 완료 되었을 경우 처리
    else if (this->m_objectStatus == IOwObject::STATUS_DONE) {
        this->m_actionTimer.stop();
        this->m_actionName = IOwObject::GetMetaEnum(IOwObject::OBJECT_MOVE_DIRECTION::DIRECTION_S);
        this->m_objectStatus = IOwObject::STATUS_NONE;
        this->m_animationNo = 0;
        return;
    }

    // 움직임 처리
    else {
        // 방향을 찾는다.
        GetDirection(this->m_movePointPixelOld.x(),
                     this->m_movePointPixelOld.y(),
                     this->m_movePointPixel.x(),
                     this->m_movePointPixel.y());

        // 아무것 안하는 상태일 경우 DIRECTION_S 방향을 보도록 한다.
        if (this->m_actionName
            == IOwObject::GetMetaEnum(IOwObject::OBJECT_MOVE_DIRECTION::DIRECTION_NONE)) {
            this->m_actionName = IOwObject::GetMetaEnum(
                IOwObject::OBJECT_MOVE_DIRECTION::DIRECTION_S);
        }

        // 현재 좌표를 이전좌표로 백업한다.
        this->m_movePointPixelOld = this->m_movePointPixel;

        // 현재 좌표가 목적지 보다 좌측에 있을 경우
        if (this->m_moveEndPointPixel.x() > this->m_movePointPixel.x()) {
            this->m_movePointPixel.setX(this->m_movePointPixel.x()
                                        + this->m_splitObjectInfo[this->m_actionName]->movePixel);
        }

        // 현재 좌표가 목적지 보다 우측에 있을 경우
        else if (this->m_moveEndPointPixel.x() < this->m_movePointPixel.x()) {
            this->m_movePointPixel.setX(this->m_movePointPixel.x()
                                        - this->m_splitObjectInfo[this->m_actionName]->movePixel);
        }

        // 현재 좌표가 목적지 보다 아래에 있을 경우
        if (this->m_moveEndPointPixel.y() > this->m_movePointPixel.y()) {
            this->m_movePointPixel.setY(this->m_movePointPixel.y()
                                        + this->m_splitObjectInfo[this->m_actionName]->movePixel);
        }

        // 현재 좌표가 목적지 보다 위에 있을 경우
        else if (this->m_moveEndPointPixel.y() < this->m_movePointPixel.y()) {
            this->m_movePointPixel.setY(this->m_movePointPixel.y()
                                        - this->m_splitObjectInfo[this->m_actionName]->movePixel);
        }

        // 현재 좌표가 목적지와 같을 경우
        if (this->m_movePointPixel.x() == this->m_moveEndPointPixel.x()
            && this->m_movePointPixel.y() == this->m_moveEndPointPixel.y()) {
            // 현재 상태를 목적지 도착으로 변경하고 목적지 도착 행동을 하도록 처리한다.
            this->m_destination = IOwObject::DESTINATION_ARRIVED;
            this->m_objectStatus = IOwObject::STATUS_ARRIVED;
            this->m_actionName = IOwObject::GetMetaEnum(IOwObject::OBJECT_STATUS::STATUS_ARRIVED);
            this->m_animationNo = 0;
        }
    }
}

/*!
 * \brief OwObject::GetDirection 오브젝트가 움직이는 방향을 계산한다.
 * \param moveOldX
 * \param moveOldY
 * \param moveX
 * \param moveY
 */
void OwObject::GetDirection(int moveOldX, int moveOldY, int moveX, int moveY)
{
    int moveDirection;

    if (moveOldY > moveY) {
        moveDirection = IOwObject::DIRECTION_N;
    } else if (moveOldY < moveY) {
        moveDirection = IOwObject::DIRECTION_S;
    } else {
        moveDirection = IOwObject::DIRECTION_NONE;
    }

    if (moveOldX > moveX) {
        moveDirection |= IOwObject::DIRECTION_W;
    } else if (moveOldX < moveX) {
        moveDirection |= IOwObject::DIRECTION_E;
    }

    auto metaEnum = QMetaEnum::fromType<IOwObject::OBJECT_MOVE_DIRECTION>();
    this->m_actionName = metaEnum.valueToKey(moveDirection);
}

void OwObject::CreateObject() {}

/*!
 * \brief OwObject::DrawObject 오브젝트를 그린다.
 * \param painter
 */
void OwObject::DrawObject(QPainter *painter)
{
    if (this->m_objectStatus == IOwObject::STATUS_NONE) {
        this->m_animationNo = 0;
    }

    if (this->m_splitObjectInfo.count() > 0) {
        // 오브젝트의 바닥이 매트릭 중앙에 오도록 보정한다. (케릭터일 경우와 사물일 경우가 다른데...흠)
        int bottomX, bottomY;
        bottomX = (this->m_isometric->defaultIsometricWidth()
                   - this->m_splitObjectInfo[this->m_actionName]->bottom.x());
        bottomY = (this->m_isometric->defaultIsometricHeight()
                   - this->m_splitObjectInfo[this->m_actionName]->bottom.y());

        painter->drawPixmap(this->m_movePointPixel.x() + bottomX,
                            this->m_movePointPixel.y() - bottomY,
                            this->m_splitObjectInfo[this->m_actionName]
                                ->splitImage[this->m_animationNo]);

        this->m_animationNo++;

        if (this->m_animationNo >= this->m_splitObjectInfo[this->m_actionName]->totalSplitCount) {
            this->m_animationNo = 0;
        }
    }
}

/*!
 * \brief OwObject::MouseDown 마우스가 눌렸을 경우 오브젝트의 시작좌표와 도착좌표를 계산한다.
 * \param metricX
 * \param metricY
 */
void OwObject::MouseDown(int metricX, int metricY)
{
    if (this->m_objectStatus == IOwObject::STATUS_NONE) {
        this->m_objectStatus = IOwObject::STATUS_WORK;
        this->m_movePointPixelOld = this->m_movePointPixel;
    } else if (this->m_objectStatus == IOwObject::STATUS_WORK) {
        this->m_moveEndPoint.setX(metricX);
        this->m_moveEndPoint.setY(metricY);
        this->m_moveEndPointPixel = this->m_isometric->GetMetricPixel(metricX, metricY);

        this->m_animationNo = 0;
        this->m_destination = IOwObject::DESTINATION_START;

        this->m_actionTimer.start(OWDrawWidget::RENDERING_TIME);
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

void OwObject::MouseHover(int metricX, int metricY)
{
    Q_UNUSED(metricX)
    Q_UNUSED(metricY)
}

/*!
 * \brief OwObject::SetObjectSplitImageInfo 분할 이미지를 만든다.
 * \param objInfo
 */
void OwObject::SetObjectSplitImageInfo(QMap<QString, ObjectSplitImageInfo *> objInfo)
{
    this->m_splitObjectInfo = objInfo;
    CreateSplitImage();
}

/*!
 * \brief OwObject::CreateSplitImage 원본 이미지에서 동작 이미지 추출하여 Map으로 만든다.
 */
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
                // 우측방향으로 복사
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
                // 아래 방향으로 복사
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
