#ifndef RAYDISPLAYSCENE_H
#define RAYDISPLAYSCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QVector>
#include <opencv2/core/core.hpp>

class QGraphicsEllipseItem;
class QGraphicsLineItem;
class QPolygonF;

class RayDisplayScene : public QGraphicsScene
{
	Q_OBJECT
	Q_PROPERTY( bool collisionEnabled READ isCollisionEnabled WRITE setCollisionEnabled )

	struct Sender
	{
		QGraphicsEllipseItem *r;
		int rotation;
		int side;
	};

	struct Ray
	{
		QLineF line;
		bool visible;
		bool isCornerRay;
	};

public:
	explicit RayDisplayScene(QObject *parent = 0);
	virtual ~RayDisplayScene();
	void initLeds();
	void lightenSender(int senderId, const int &angle);
	int sendersCount() const;
	bool isCollisionEnabled() const;
    void updateCollisions();

signals:
	
public slots:
	void initRays(const int &angle);
	void clearRays();
	void clearObstacle();
	void setCollisionEnabled(bool enable);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
	bool isStartingRay(const QVector<Ray> &rays, const int idx) const;
	bool isCornerRay(const QVector<Ray> &rays, const int idx) const;
	bool isFinishingRay(const QVector<Ray> &rays, const int idx) const;
	QVector<Sender> mSenders;
	QVector<QGraphicsEllipseItem *> mReceivers;
	QVector<QVector<QGraphicsEllipseItem *> > mSidedReceivers;
	QVector<QGraphicsLineItem *> mRays;
	QPolygonF mObstacle;
	QGraphicsPolygonItem *mGraphicsObstacle;
	QVector<QVector<QLineF> > mCollidedRays;
	bool mCollisionEnabled;
    QVector<QGraphicsEllipseItem *> mCollisions;
	QVector<QVector<QGraphicsLineItem *> > mCollidedRaysGraphics;
	QVector<QList<QGraphicsPolygonItem *> > mTriangles;
	QVector<QGraphicsTextItem *> mRayNumbers;
	QVector<QList<QVector<cv::Point2i> > > mCvPolygons;
	QVector<cv::Mat> mMats;
};

#endif // RAYDISPLAYSCENE_H
