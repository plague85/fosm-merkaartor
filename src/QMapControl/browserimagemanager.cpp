/***************************************************************************
 *   Copyright (C) 2008 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "browserimagemanager.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QApplication>
#include <QDateTime>
#include <QPixmapCache>
#include <QPainter>
#include <QMessageBox>
#include <QCryptographicHash>

#define MAX_REQ 1
#define BROWSER_TILE_SIZE 512
#define str(x) # x

void BrowserWebPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID )
{
	Q_UNUSED(message)
	Q_UNUSED(lineNumber)
	Q_UNUSED(sourceID)
	// FIXME Seems to crash at times (see http://trac.openstreetmap.org/ticket/1194)
	//QString s = QString("%1 at %2, %3").arg(message).arg(QString::number(lineNumber)).arg(sourceID);
	//printf("%s\n", s);
}

void BrowserWebPage::javaScriptAlert ( QWebFrame * frame, const QString & msg ) 
{
	Q_UNUSED(frame)
	//QMessageBox::information(NULL, tr("Javascript alert"), msg);

	if (msg.startsWith("Coord")) {

		tllat = 90.0;
		tllon = -180.0;
		brlat = -90.0;
		brlon = 180.0;
		
		QStringList tokens = msg.split(" ");
		Q_ASSERT(tokens.size() == 5);

		tllat = tokens[1].toDouble();
		tllon = tokens[2].toDouble();
		brlat = tokens[3].toDouble();
		brlon = tokens[4].toDouble();

		qDebug() << tllon << ", " << tllat << ", " << brlon << "," << brlat;
	} else
	if (msg.startsWith("Size")) {

		sw = BROWSER_TILE_SIZE;
		sh = BROWSER_TILE_SIZE;

		QStringList tokens = msg.split(" ");
		Q_ASSERT(tokens.size() == 5);

		int ox, oy, x1, y1;
		ox = int(tokens[1].toDouble());
		oy = int(tokens[2].toDouble());
		x1 = int(tokens[3].toDouble());
		y1 = int(tokens[4].toDouble());

		qDebug() << ox << ", " << oy << ", " << x1 << "," << y1;

		sw = x1 - ox;
		sh = y1 - oy;

		qDebug() << sw << ", " << sh;
	}
}


BrowserImageManager* BrowserImageManager::m_BrowserImageManagerInstance = 0;

BrowserImageManager::BrowserImageManager(QObject* parent)
	:QObject(parent), emptyPixmap(QPixmap(1,1)), browser(0), requestActive(false)
{
	emptyPixmap.fill(Qt::transparent);

	if (QPixmapCache::cacheLimit() <= 20000)
	{
		QPixmapCache::setCacheLimit(20000);	// in kb
	}

	browser = new QWebView();
	page = new BrowserWebPage();
	browser->setPage(page);
	page->setViewportSize(QSize(1024, 1024));

#ifndef QT_WEBKIT_LIB
	connect(page->mainFrame(), SIGNAL(loadDone(bool)), this, SLOT(pageLoadFinished(bool)));
#endif
	connect(page, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadFinished(bool)), Qt::QueuedConnection);
}

BrowserImageManager::~BrowserImageManager()
{
	delete browser;
}

//QPixmap BrowserImageManager::getImage(const QString& host, const QString& url)
QPixmap BrowserImageManager::getImage(IMapAdapter* anAdapter, int x, int y, int z)
{
// 	qDebug() << "BrowserImageManager::getImage";
	QPixmap pm(emptyPixmap);

	QString host = anAdapter->getHost();
	QString url = anAdapter->getQuery(x, y, z);
	QString strHash = QString("%1;%2;%3;%4;%5").arg(anAdapter->getName()).arg(QString::number(x)).arg(QString::number(y)).arg(QString::number(z)).arg(anAdapter->getTileSize());
	QString hash = QString(strHash.toAscii().toBase64());
	if (hash.size() > 255) {
		QCryptographicHash crypt(QCryptographicHash::Md5);
		crypt.addData(hash.toLatin1());
		hash = QString(crypt.result().toHex());
	}

	// is image in picture cache
	if (QPixmapCache::find(hash, pm))
		return pm;

	// disk cache?
    if (useDiskCache(hash + ".png")) {
		if (pm.load(cacheDir.absolutePath() + "/" + hash + ".png")) {
			QPixmapCache::insert(hash, pm);
			return pm;
		}
	}
	if (M_PREFS->getOfflineMode())
		return pm;

	LoadingRequest LR(hash, host, url);
	if (loadingRequests.contains(LR))
		return pm;

	loadingRequests.enqueue(LR);
	emit(imageRequested());

	if (!requestActive)
		launchRequest();
	else {
		//qDebug() << "queue full";
		return emptyPixmap;
	}

	return pm;
}

void BrowserImageManager::launchRequest()
{
	if (loadingRequests.isEmpty())
		return;
//	LoadingRequest* R = loadingRequests.dequeue();
	LoadingRequest R = loadingRequests.head();
	qDebug() << "getting: " << QString(R.host).append(R.url);


	QUrl u = QUrl( R.url);

#if QT_VERSION >= 0x040400
	page->networkAccessManager()->setProxy(proxy);
#else
	page->setNetworkProxy(proxy);
#endif

//	page->mainFrame()->load(u);
	browser->load(u);
	requestActive = true;
}

void BrowserImageManager::pageLoadFinished(bool)
{
	QPixmap pt(page->sw, page->sh);
	QPainter P;
	P.begin(&pt);
	page->mainFrame()->render(&P, QRegion(0,0,page->sw,page->sh));
	P.end();
	
	if (page->sw != BROWSER_TILE_SIZE || page->sh != BROWSER_TILE_SIZE) {
		QPixmap tmpPx = pt.scaled(QSize(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
		pt = tmpPx;
	}

	LoadingRequest R = loadingRequests.dequeue();
	//pt.save("c:/temp/tst/"+R.hash+".png");
	receivedImage(pt, R.hash);
	requestActive = false;

	if (loadingRequests.isEmpty()) {
		loadingQueueEmpty();
	} else
		launchRequest();
}

void BrowserImageManager::slotLoadProgress(int p)
{
	if (!(p < 100)) {
	}
}

//QPixmap BrowserImageManager::prefetchImage(const QString& host, const QString& url)
QPixmap BrowserImageManager::prefetchImage(IMapAdapter* anAdapter, int x, int y, int z)
{
	QString host = anAdapter->getHost();
	QString url = anAdapter->getQuery(x, y, z);
	QString strHash = QString("%1;%2;%3;%4;%5").arg(anAdapter->getName()).arg(QString::number(x)).arg(QString::number(y)).arg(QString::number(z)).arg(anAdapter->getTileSize());
	QString hash = QString(strHash.toAscii().toBase64());

	prefetch.append(hash);
	return getImage(anAdapter, x, y, z);
}

void BrowserImageManager::receivedImage(const QPixmap& pixmap, const QString& hash)
{
// 	qDebug() << "BrowserImageManager::receivedImage";
	if (pixmap.isNull()) {
		QPixmap pm(256, 256);
		pm.fill(Qt::gray);
		QPixmapCache::insert(hash, pm);
	} else {
		QPixmapCache::insert(hash, pixmap);
		QString strHash = QByteArray::fromBase64(hash.toAscii());

		if (cacheMaxSize && !strHash.startsWith("Yahoo")) {
			pixmap.save(cacheDir.absolutePath() + "/" + hash + ".png");
			QFileInfo info(cacheDir.absolutePath() + "/" + hash + ".png");
			cacheInfo.append(info);
			cacheSize += info.size();

			adaptCache();
		}
	}

	if (prefetch.contains(hash))
	{
		prefetch.remove(prefetch.indexOf(hash));
	}
	emit(imageReceived());
}

void BrowserImageManager::loadingQueueEmpty()
{
	emit(loadingFinished());
// 	((Layer*)this->parent())->removeZoomImage();
// 	qDebug() << "size of image-map: " << images.size();
// 	qDebug() << "size: " << QPixmapCache::cacheLimit();
}

void BrowserImageManager::abortLoading()
{
	if (!loadingRequests.isEmpty()) {
		LoadingRequest R = loadingRequests.dequeue();
		loadingRequests.clear();
		loadingRequests.enqueue(R);
	}
}

void BrowserImageManager::setProxy(QString host, int port)
{
	if (!host.isEmpty()) {
#if QT_VERSION >= 0x040400
		proxy.setType(QNetworkProxy::HttpCachingProxy);
#else
		proxy.setType(QNetworkProxy::HttpProxy);
#endif
		proxy.setHostName(host);
		proxy.setPort(port);
	}
}
