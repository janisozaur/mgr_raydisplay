#include "RayDisplayScene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <opencv2/highgui/highgui.hpp>
#include <QThread>

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
	QList<int> sizes;
	//sizes << 64 << 40 << 64 << 40;
	sizes << 48 << 32 << 48 << 32;
	Q_ASSERT(4 == sizes.size());
	mSidedReceivers.resize(4);
	int allReceivers = 0;
	for (int i = 0; i < sizes.size(); i++) {
		Q_ASSERT(sizes.at(i) % 8 == 0);
		mSidedReceivers[i].reserve(sizes.at(i));
		allReceivers += sizes.at(i) / 8;
	}
	mReceivers.reserve(allReceivers * 8);

	mSenders.reserve(allReceivers);
	// top left-to-right
	for (int i = 0; i < sizes.at(0); i++) {
        QGraphicsEllipseItem *r;
        const int x = i * 10;
		const int y = 0;
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::black), 2));
		r->setPos(x, y);
		mReceivers.append(r);
		mSidedReceivers[0] << r;
	}
    // right top-to-bottom
	for (int i = 0; i < sizes.at(1); i++) {
        QGraphicsEllipseItem *r;
		const int x = sizes.at(0) * 10;
		const int y = i * 10 - (0 - 5);
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::black), 2));
		r->setPos(x, y);
        mReceivers.append(r);
		mSidedReceivers[1] << r;
    }
    // bottom right-to-left
	for (int i = 0; i < sizes.at(2); i++) {
        QGraphicsEllipseItem *r;
		const int x = i * -10 + sizes.at(0) * 10 - 10;
		const int y = sizes.at(1) * 10;
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::black), 2));
		r->setPos(x, y);
        mReceivers.append(r);
		mSidedReceivers[2] << r;
    }
    // left bottom-to-top
	for (int i = 0; i < sizes.at(3); i++) {
        QGraphicsEllipseItem *r;
        const int x = -5;
		const int y = i * -10 + ((sizes.at(1)) * 10 - 5);
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::black), 2));
		r->setPos(x, y);
        mReceivers.append(r);
		mSidedReceivers[3] << r;
    }

    // top left-to-right
	for (int i = 0; i < (sizes.at(0) / 8); i++) {
        QGraphicsEllipseItem *r;
        const int x = i * 80 + 35;
		const int y = 0;
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::red), 2));
		r->setPos(x, y);
		mSenders.append(Sender{r, 270, 0});
    }
    // right top-to-bottom
	for (int i = 0; i < (sizes.at(1) / 8); i++) {
		QGraphicsEllipseItem *r;
		const int x = sizes.at(0) * 10;
		const int y = i * 80 - (0 - 40);
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::red), 2));
		r->setPos(x, y);
		mSenders.append(Sender{r, 180, 1});
	}
    // bottom right-to-left
	for (int i = 0; i < (sizes.at(2) / 8); i++) {
        QGraphicsEllipseItem *r;
		const int x = i * -80 + (sizes.at(0) * 10 - 40 - 5);
		const int y = sizes.at(1) * 10;
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::red), 2));
		r->setPos(x, y);
		mSenders.append(Sender{r, 90, 2});
    }
    // left bottom-to-top
	for (int i = 0; i < (sizes.at(3) / 8); i++) {
		QGraphicsEllipseItem *r;
        const int x = -5;
		const int y = i * -80 + ((sizes.at(1) * 10) - 40);
		r = addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::red), 2));
		r->setPos(x, y);
        //QGraphicsSimpleTextItem *t = addSimpleText(QString::number(i));
        //t->moveBy(x + 10, y);
		mSenders.append(Sender{r, 0, 3});
	}
	mCollidedRays.clear();
	mCollidedRays.resize(mSenders.size());
	mCollidedRaysGraphics.clear();
	mCollidedRaysGraphics.resize(mSenders.size());
	mTriangles.clear();
	mTriangles.resize(mSenders.size());
	mMats.reserve(mSenders.size());
	for (int i = 0; i < mSenders.size(); i++) {
		mMats << cv::Mat(sizes.at(1) * 10, sizes.at(0) * 10, CV_8U);
	}
	/*QVector<cv::Point> points;
	points << cv::Point2i(0, 0);
	points << cv::Point2i(0, 100);
	points << cv::Point2i(100, 0);
	const cv::Point *pointsPtr[1] = {points.data()};
	int size = points.size();
	cv::Scalar color = cv::Scalar(127);
	cv::Mat m = mMats[0];
	cv::line(m, cv::Point2i(0, 0), cv::Point2i(100, 100), color, 22);
	cv::imshow("plepleple", m);
	cv::fillConvexPoly(m, points.data(), size, color);
	qDebug() << "before";
	T::sleep(4);
	qDebug() << "after";
	cv::imshow("plepleple", m);*/
}

void RayDisplayScene::lightenSender(int senderId, const int &angle)
{
	for (int i = 0; i < mRayNumbers.size(); i++) {
		delete mRayNumbers.at(i);
	}
	mRayNumbers.resize(0);
	QVector<QLineF> &senderCollidedRays = mCollidedRays[senderId];
    if (mCollisionEnabled) {
		for (int i = 0; i < mCollidedRaysGraphics.at(senderId).size(); i++) {
			delete mCollidedRaysGraphics.at(senderId).at(i);
		}
		mCollidedRaysGraphics[senderId].resize(0);
		senderCollidedRays.resize(0);
	}
	for (int i = 0; i < mTriangles.size(); i++) {
		for (int j = 0; j < mTriangles.at(i).size(); j++) {
			delete mTriangles.at(i).at(j);
		}
		mTriangles[i].clear();
	}
	QVector<Ray> senderRays;
	senderRays.reserve(10);
	for (int i = 1; i < mSidedReceivers.size(); i++) {
		const int sideIdx = (mSenders.at(senderId).side + i) % mSidedReceivers.size();
		for (int j = 0; j < mSidedReceivers.at(sideIdx).size(); j++) {
			QLineF line(mSenders.at(senderId).r->pos(), mSidedReceivers.at(sideIdx).at(j)->pos());
			int lineAngle = int(line.angle()) - mSenders.at(senderId).rotation;
			if ((lineAngle > -angle / 2 && lineAngle < angle / 2) || (lineAngle - 360 > -angle / 2 && lineAngle - 360 < angle / 2)) {
				QGraphicsTextItem *rayNumber;
				rayNumber = addText(QString::number(senderRays.size()));
				rayNumber->setPos(line.p2());
				rayNumber->setZValue(3);
				mRayNumbers << rayNumber;
				senderRays << Ray{line, true, (j == 0) || (j == mSidedReceivers.at(sideIdx).size() - 1) /* corner ray */};
			}
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
	cv::Mat cvImage(mMats.at(0).size(), CV_8U);// = mMats[senderId];
	cvImage = cv::Scalar(0);
	for (int i = 0; i < senderRays.size(); i++) {
		if (isStartingRay(senderRays, i)) {
			QPolygonF polygon;
			polygon.reserve(5);
			polygon << mSenders.at(senderId).r->pos();
			polygon << senderRays.at(i).line.p2();
			int j;
			for (j = i; j < senderRays.size(); j++) {
				if (isFinishingRay(senderRays, j)) {
					polygon << senderRays.at(j).line.p2();
					i = j + 1;
					break;
				} else if (isCornerRay(senderRays, j)) {
					polygon << senderRays.at(j).line.p2();
				}
			}
			QGraphicsPolygonItem *polyItem = addPolygon(polygon, QPen(QBrush(Qt::green), 2), QBrush(Qt::magenta));
			mTriangles[senderId] << polyItem;
			QVector<cv::Point2i> cvPoints;
			cvPoints.reserve(polygon.size());
			for (int j = 0; j < polygon.size(); j++) {
				cvPoints << cv::Point2i(polygon.at(j).x(), polygon.at(j).y());
			}
			cv::fillConvexPoly(cvImage, cvPoints.constData(), cvPoints.size(), cv::Scalar(255));
		}
	}
	mMats[senderId] = cvImage;
	//cv::imshow(QString(QString("plepleple ")/* + QString::number(senderId)*/).toStdString(), cvImage);
	/*for (int i = 0; i < mCvPolygons.size(); i++) {
		for (int j = 0; j < mCvPolygons.at(i).size(); j++) {
			cv::fillConvexPoly(cvImage, cvPoints.constData(), cvPoints.size(), cv::Scalar(127));
		}
	}*/
	cv::Mat finalImg(cvImage.size(), cvImage.type());
	finalImg = cv::Scalar(0);
	for (int i = 0; i < mMats.size(); i++) {
		//cv::bitwise_or(finalImg, mMats.at(i), finalImg);
		cv::max(finalImg, mMats.at(i), finalImg);
		//finalImg |= mMats.at(i);
	}
	cv::imshow(QString(QString("plepleple ")/* + QString::number(senderId)*/).toStdString(), finalImg);
    updateCollisions();
}

bool RayDisplayScene::isStartingRay(const QVector<Ray> &rays, const int idx) const
{
	Q_ASSERT(idx >= 0);
	Q_ASSERT(idx < rays.size());
	if (Q_UNLIKELY(0 == idx)) {
		return rays.at(idx).visible;
	} else {
		return (!rays.at(idx - 1).visible && rays.at(idx).visible);
	}
}

bool RayDisplayScene::isCornerRay(const QVector<Ray> &rays, const int idx) const
{
	Q_ASSERT(idx >= 0);
	Q_ASSERT(idx < rays.size());
	return rays.at(idx).isCornerRay;
}

bool RayDisplayScene::isFinishingRay(const QVector<Ray> &rays, const int idx) const
{
	Q_ASSERT(idx >= 0);
	Q_ASSERT(idx < rays.size());
	if (Q_UNLIKELY(rays.size() - 1 == idx)) {
		return rays.at(idx).visible;
	} else {
		return (!rays.at(idx + 1).visible && rays.at(idx).visible);
	}
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
						QGraphicsEllipseItem *collisionGraphicsItem;
						collisionGraphicsItem = addEllipse(QRectF(collisionPoint, QSizeF(2, 2)), QPen(QBrush(Qt::magenta), 2));
						mCollisions << collisionGraphicsItem;
                    }
                }
            }
        }
	}
}
