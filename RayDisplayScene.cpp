#include "RayDisplayScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>

#include <QDebug>

RayDisplayScene::RayDisplayScene(QObject *parent) :
	QGraphicsScene(parent), mCollisionEnabled(false)
{
	mGraphicsObstacle = addPolygon(mObstacle, QPen(QBrush(Qt::green), 2));
	initLeds();
}

RayDisplayScene::~RayDisplayScene()
{
}

void RayDisplayScene::initLeds()
{
	mReceivers.reserve(64 * 2 + 40 * 2);
    mSenders.reserve(2 * 8 + 2 * 5);
    // top left-to-right
    for (int i = 0; i < 64; i++) {
        QGraphicsEllipseItem *r;
        const int x = i * 10;
        const int y = -200;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::black), 2));
        mReceivers.append(r);
    }
    // right top-to-bottom
    for (int i = 0; i < 40; i++) {
        QGraphicsEllipseItem *r;
        const int x = 640;
        const int y = i * 10 - 195;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::black), 2));
        mReceivers.append(r);
    }
    // bottom right-to-left
    for (int i = 0; i < 64; i++) {
        QGraphicsEllipseItem *r;
        const int x = i * -10 + 630;
        const int y = 200;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::black), 2));
        mReceivers.append(r);
    }
    // left bottom-to-top
    for (int i = 0; i < 40; i++) {
        QGraphicsEllipseItem *r;
        const int x = -5;
        const int y = i * -10 + 195;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::black), 2));
        mReceivers.append(r);
    }

    // top left-to-right
    for (int i = 0; i < 8; i++) {
        QGraphicsEllipseItem *r;
        const int x = i * 80 + 35;
        const int y = -200;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::red), 2));
        mSenders.append(Sender{r, 270});
    }
    // right top-to-bottom
	for (int i = 0; i < 5; i++) {
		QGraphicsEllipseItem *r;
        const int x = 640;
        const int y = i * 80 - 160;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::red), 2));
		mSenders.append(Sender{r, 180});
	}
    // bottom right-to-left
    for (int i = 0; i < 8; i++) {
        QGraphicsEllipseItem *r;
        const int x = i * -80 + 595;
        const int y = 200;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::red), 2));
        mSenders.append(Sender{r, 90});
    }
    // left bottom-to-top
	for (int i = 0; i < 5; i++) {
		QGraphicsEllipseItem *r;
        const int x = -5;
        const int y = i * -80 + 160;
        r = addEllipse(x, y, 5, 5, QPen(QBrush(Qt::red), 2));
        //QGraphicsSimpleTextItem *t = addSimpleText(QString::number(i));
        //t->moveBy(x + 10, y);
		mSenders.append(Sender{r, 0});
	}
	mCollidedRays.clear();
	mCollidedRays.resize(mSenders.size());
	mCollidedRaysGraphics.clear();
	mCollidedRaysGraphics.resize(mSenders.size());
}

void RayDisplayScene::lightenSender(int senderId, const int &angle)
{
	QVector<QLineF> &senderCollidedRays = mCollidedRays[senderId];
    if (mCollisionEnabled) {
		for (int i = 0; i < mCollidedRaysGraphics.at(senderId).size(); i++) {
			delete mCollidedRaysGraphics.at(senderId).at(i);
		}
		mCollidedRaysGraphics[senderId].resize(0);
		senderCollidedRays.resize(0);
    }
	QVector<Ray> senderRays;
	senderRays.reserve(10);
	for (int j = 0; j < mReceivers.size(); j++) {
		QLineF line(mSenders.at(senderId).r->rect().topLeft(), mReceivers.at(j)->rect().topLeft());
		int lineAngle = int(line.angle()) - mSenders.at(senderId).rotation;
		if ((lineAngle > -angle / 2 && lineAngle < angle / 2) || (lineAngle - 360 > -angle / 2 && lineAngle - 360 < angle / 2)) {
			senderRays << Ray{line, true};
		}
	}
	for (int i = 0; i < senderRays.size(); i++) {
		QGraphicsLineItem *r = nullptr;
		if (!mCollisionEnabled || mObstacle.size() <= 1) {
			r = addLine(senderRays.at(i).line, QPen(QBrush(Qt::blue), 1));
		} else {
			const int size = mObstacle.size();
			for (int j = 0; j < size; j++) {
				QLineF obsLine(mObstacle.at(j), mObstacle.at((j + 1) % size));
				if (senderRays.at(i).line.intersect(obsLine, nullptr) == QLineF::BoundedIntersection) {
					senderRays[i].visible = false;
					break;
				}
			}
			if (senderRays.at(i).visible) {
				r = addLine(senderRays.at(i).line, QPen(QBrush(Qt::blue), 1));
			}
		}
		if (r != nullptr) {
			mRays.append(r);
		}
	}
	if (senderRays.size() > 0) {
		QGraphicsLineItem *r = nullptr;
		/*if (!senderRays.at(0).visible)*/ {
			//senderCollidedRays << senderRays.at(0).line;
			r = addLine(senderRays.at(0).line, QPen(QBrush(Qt::yellow), 1));
			mRays.append(r);
		}
		if (senderRays.size() > 1/* && !senderRays.at(senderRays.size() - 1).visible*/) {
			//senderCollidedRays << senderRays.at(senderRays.size() - 1).line;
			r = addLine(senderRays.at(senderRays.size() - 1).line, QPen(QBrush(Qt::yellow), 1));
			mRays.append(r);
		}
		for (int i = 1; i < senderRays.size() - 1; i++) {
			if (!senderRays.at(i).visible && (senderRays.at(i - 1).visible || senderRays.at(i + 1).visible)) {
				senderCollidedRays << senderRays.at(i).line;
				r = addLine(senderRays.at(i).line, QPen(QBrush(Qt::red), 1));
				mCollidedRaysGraphics[senderId] << r;
			}
		}
	}
    updateCollisions();
}

void RayDisplayScene::initRays(const int &angle)
{
	for (int i = 0; i < mSenders.size(); i++) {
		lightenSender(i, angle);
	}
}

void RayDisplayScene::clearRays()
{
	for (int i = 0; i < mRays.size(); i++) {
		delete mRays.at(i);
	}
	mRays.resize(0);
}

void RayDisplayScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::LeftButton)) {
        qDebug() << "left" << event->pos() << event->scenePos();
	}
	if (event->buttons().testFlag(Qt::RightButton)) {
        qDebug() << "right" << event->pos() << event->scenePos();
		mObstacle << event->scenePos();
		mGraphicsObstacle->setPolygon(mObstacle);
	}
}

void RayDisplayScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	const int x = event->scenePos().x();
	const int y = event->scenePos().y();
	for (int i = 0; i < mRays.size(); i++) {
		const float x2x1 = mRays.at(i)->line().p2().x() - mRays.at(i)->line().p1().x();
		const float y2y1 = mRays.at(i)->line().p2().y() - mRays.at(i)->line().p1().y();
		const float d = (x2x1) * (mRays.at(i)->line().p1().y() - y) - (mRays.at(i)->line().p1().x() - x) * (y2y1);
		const float dSq = d * d;
		const float y2y1Sq = y2y1 * y2y1;
		const float x2x1Sq = x2x1 * x2x1;
		const float distSq = dSq / (x2x1Sq + y2y1Sq);
		const bool visible = distSq > 4;
		if (!(mRays.at(i)->isVisible() & visible)) {
			mRays[i]->setVisible(visible);
		}
	}
}

void RayDisplayScene::clearObstacle()
{
	mObstacle.clear();
	mGraphicsObstacle->setPolygon(mObstacle);
}

int RayDisplayScene::sendersCount() const
{
	return mSenders.count();
}

void RayDisplayScene::setCollisionEnabled(bool enable)
{
	mCollisionEnabled = enable;
}

bool RayDisplayScene::isCollisionEnabled() const
{
	return mCollisionEnabled;
}

void RayDisplayScene::updateCollisions()
{
    QPointF collisionPoint;
    for (int i = 0; i < mCollisions.size(); i++) {
        delete mCollisions.at(i);
    }
	QGraphicsEllipseItem *collisionGraphicsItem;
	mCollisions.resize(0);
    for (int i = 0; i < mCollidedRays.size(); i++) {
		const int jCount = mCollidedRays.at(i).size();
        for (int k = 0; k < mCollidedRays.size(); k++) {
            if (i == k) {
                continue;
            }
			const int lCount = mCollidedRays.at(k).size();
            for (int j = 0; j < jCount; j++) {
                for (int l = 0; l < lCount; l++) {
					if (mCollidedRays.at(i).at(j).intersect(mCollidedRays.at(k).at(l), &collisionPoint) == QLineF::BoundedIntersection) {
						collisionGraphicsItem = addEllipse(QRectF(collisionPoint, QSizeF(2, 2)), QPen(QBrush(Qt::magenta), 2));
						mCollisions << collisionGraphicsItem;
                    }
                }
            }
        }
	}
}
