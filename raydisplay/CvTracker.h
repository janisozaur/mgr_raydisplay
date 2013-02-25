#ifndef CVTRACKER_H
#define CVTRACKER_H

#include "TrackerInterface.h"
#include "Tracker.h"

#include <QVector>
#include <opencv2/core/core.hpp>

class CvTracker : public TrackerInterface
{
	Q_OBJECT
public:
	explicit CvTracker(const QVector<QVector<QPointF>> &receivers, const QVector<QVector<QPointF>> &senders, QObject *parent);
	QVariantMap trackBlobs(const QVector<RayStatus> rays, const int senderId);
	
private:
	QVector<cv::Mat> mMats;
	Tracker mTracker;
};

#endif // CVTRACKER_H
