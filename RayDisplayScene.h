#ifndef RAYDISPLAYSCENE_H
#define RAYDISPLAYSCENE_H

#include <QGraphicsScene>
#include <QList>

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
	};

	struct Ray
	{
		QLineF line;
		bool visible;
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
	QVector<Sender> mSenders;
	QVector<QGraphicsEllipseItem *> mReceivers;
	QVector<QGraphicsLineItem *> mRays;
	QPolygonF mObstacle;
	QGraphicsPolygonItem *mGraphicsObstacle;
	QVector<QVector<QLineF> > mCollidedRays;
	bool mCollisionEnabled;
    QVector<QGraphicsEllipseItem *> mCollisions;
	QVector<QVector<QGraphicsLineItem *> > mCollidedRaysGraphics;
};

#endif // RAYDISPLAYSCENE_H
