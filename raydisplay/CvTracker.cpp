#include "CvTracker.h"

#include <QPolygonF>
#include <opencv2/highgui/highgui.hpp>
#include <QDebug>

CvTracker::CvTracker(const QVector<QVector<QPointF>> &receivers, const QVector<QVector<QPointF>> &senders, QObject *parent) :
	TrackerInterface(receivers, senders, parent),
	mTracker(Tracker(0, 1000))
{
	const int size = count(senders);
	mMats.reserve(size);
	for (int i = 0; i < size; i++) {
		// width * 10, height * 10
		mMats << cv::Mat(senders.at(1).size() * 10, senders.at(0).size() * 10, CV_8U);
	}
}

QVariantMap CvTracker::trackBlobs(const QVector<RayStatus> rays, const int senderId)
{
	cv::Mat cvImage(mMats.at(0).size(), CV_8U);// = mMats[senderId];
	cvImage = cv::Scalar(255);
	for (int i = 0, n = rays.size(); i < n; i++) {
		if (isStartingRay(rays, i)) {
			qDebug() << __func__ << "starting ray" << i;
			QPolygonF polygon;
			polygon.reserve(5);
			polygon << idToSender(senderId);
			polygon << idToReceiver(i);
			for (int j = i; j < n; j++) {
				if (isFinishingRay(rays, j)) {
					qDebug() << __func__ << "finishing ray" << j;
					polygon << idToReceiver(j);
					i = j + 1;
					break;
				} else if (isCornerRay(j)) {
					qDebug() << __func__ << "corner ray" << j;
					polygon << idToReceiver(j);
				}
			}
			//QGraphicsPolygonItem *polyItem = addPolygon(polygon, QPen(QBrush(Qt::green), 2), QBrush(Qt::magenta));
			//mTriangles[senderId] << polyItem;
			QVector<cv::Point2i> cvPoints;
			cvPoints.reserve(polygon.size());
			for (int j = 0; j < polygon.size(); j++) {
				cvPoints << cv::Point2i(polygon.at(j).x(), polygon.at(j).y());
			}
			cv::fillConvexPoly(cvImage, cvPoints.constData(), cvPoints.size(), cv::Scalar(0));
		}
	}
	mMats[senderId] = cvImage;



	cv::Mat finalImg(cvImage.size(), cvImage.type());
	finalImg = cv::Scalar(255);
	for (int i = 0; i < mMats.size(); i++) {
		//cv::bitwise_or(finalImg, mMats.at(i), finalImg);
		cv::min(finalImg, mMats.at(i), finalImg);
		//finalImg |= mMats.at(i);
	}
	cv::imshow(QString(QString("plepleple ")/* + QString::number(senderId)*/).toStdString(), finalImg);
	mTracker.trackBlobs(finalImg, false);
	cv::Mat blobsImg(cvImage.size(), cvImage.type());
	blobsImg = cv::Scalar(0);
	QVector<Blob> blobs = mTracker.getBlobs();
	qDebug() << "blobs.size() = " << blobs.size();
	for (int i = 0; i < blobs.size(); i++) {
		qDebug() << "blobs.at(" << i << "): min = " << blobs.at(i).min.x << ", " << blobs.at(i).min.y;
		cv::rectangle(blobsImg, blobs.at(i).min, blobs.at(i).max, cv::Scalar(255));
	}
	cv::imshow(QString(QString("blobs")/* + QString::number(senderId)*/).toStdString(), blobsImg);
	return QVariantMap();
}
