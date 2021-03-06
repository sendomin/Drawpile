/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2006-2014 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QGraphicsView>

class QGestureEvent;

#include "core/point.h"
#include "utils/strokesmoother.h"
#include "bundled/kis_cubic_curve.h"
#include "tools/tool.h"

namespace drawingboard {
	class CanvasScene;
}

namespace net {
	class Client;
}

namespace docs {
	class ToolSettings;
}

namespace widgets {


/**
 * @brief Editor view
 * The editor view is a customized QGraphicsView that displays
 * the drawing board and handes user input.
 * It also provides other features, such as brush outline preview.
 */
class CanvasView : public QGraphicsView
{
	Q_OBJECT
	public:
		enum PressureMode { PRESSUREMODE_STYLUS, PRESSUREMODE_DISTANCE, PRESSUREMODE_VELOCITY };

		CanvasView(QWidget *parent=0);

		//! Set the board to use
		void setCanvas(drawingboard::CanvasScene *scene);

		//! Set the network client
		void setClient(net::Client *client);

		//! Set the tool settings panel
		void setToolSettings(docks::ToolSettings *settings);
		
		//! Get the current zoom factor
		qreal zoom() const { return _zoom; }

		//! Get the current rotation angle in degrees
		qreal rotation() const { return _rotate; }
		
		//! Set fake pressure mode
		void setPressureMode(PressureMode mode, float param);

		using QGraphicsView::mapToScene;
		paintcore::Point mapToScene(const QPoint &point, qreal pressure) const;
		paintcore::Point mapToScene(const QPointF &point, qreal pressure) const;

		//! The center point of the view in scene coordinates
		QPoint viewCenterPoint() const;

		//! Quick adjust current tool attribute 1 if possible
		void doQuickAdjust1(float delta);

		//! Enable/disable tablet event handling
		void enableTabletEvents(bool enable);

		//! Is drawing in progress at the moment?
		bool isPenDown() const { return _pendown != NOTDOWN; }

	signals:
		//! An image has been dropped on the widget
		void imageDropped(const QImage &image);

		//! An URL was dropped on the widget
		void urlDropped(const QUrl &url);

		//! A color was dropped on the widget
		void colorDropped(const QColor &color);

		//! Viewport has changed
		void viewRectChange(const QPolygonF& viewport);

		//! The view has been transformed
		void viewTransformed(qreal zoom, qreal angle);

		//! Pointer moved in pointer tracking mode
		void pointerMoved(const QPointF &point);

	public slots:
		//! Select the active tool
		void selectTool(tools::Type tool);
		
		//! Select the currently active layer
		void selectLayer(int layer_id);
		
		//! Set the size of the brush preview outline
		void setOutlineSize(int size);

		//! Enable subpixel precision for brush preview outline
		void setOutlineSubpixelMode(bool subpixel);

		//! Enable or disable cursor outline
		void setOutline(bool enable);

		//! Scroll view to location
		void scrollTo(const QPoint& point);
		
		//! Set the zoom factor
		void setZoom(qreal zoom);

		//! Set the rotation angle in degrees
		void setRotation(qreal angle);

		void setLocked(bool lock);

		//! Send pointer position updates even when not drawing
		void setPointerTracking(bool tracking);

		//! Set stroke smoothing strength
		void setStrokeSmoothing(int smoothing);

		//! Set stylus pressure adjustment curve
		void setPressureCurve(const KisCubicCurve &curve);

		//! Set distance to pressure curve
		void setDistanceCurve(const KisCubicCurve &curve);

		//! Set velocity to pressure curve
		void setVelocityCurve(const KisCubicCurve &curve);

		//! Increase zoom factor
		void zoomin();

		//! Decrease zoom factor
		void zoomout();

	protected:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		void scrollContentsBy(int dx, int dy);
		void resizeEvent(QResizeEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void mouseDoubleClickEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void keyReleaseEvent(QKeyEvent *event);
		bool viewportEvent(QEvent *event);
		void drawForeground(QPainter *painter, const QRectF& rect);
		void dragEnterEvent(QDragEnterEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dropEvent(QDropEvent *event);

	private:
		void viewRectChanged();
		float mapPressure(float pressure, bool stylus);

		//! View transformation mode (for dragging)
		enum ViewTransform {DRAG_NOTRANSFORM, DRAG_TRANSLATE, DRAG_ROTATE, DRAG_ZOOM, DRAG_QUICKADJUST1};

		//! Start dragging the view
		void startDrag(int x, int y, ViewTransform mode);

		//! Stop dragging the view
		void stopDrag();

		//! Drag the view
		void moveDrag(int x, int y);

		//! Redraw the scene around the outline cursor if necesasry
		void updateOutline(paintcore::Point point);
		void updateOutline();

		void onPenDown(const paintcore::Point &p, bool right);
		void onPenMove(const paintcore::Point &p, bool right, bool shift, bool alt);
		void onPenUp(bool right);
		
		void gestureEvent(QGestureEvent *event);

		void resetCursor();

		/**
		 * @brief State of the pen
		 *
		 * - NOTDOWN pen is not down
		 * - MOUSEDOWN mouse is down
		 * - TABLETDOWN tablet stylus is down
		 */
		enum {NOTDOWN, MOUSEDOWN, TABLETDOWN} _pendown;

		//! If Ctrl is held, pen goes to "special" mode (which is currently quick color picker mode)
		bool _specialpenmode;

		//! Is the view being dragged
		ViewTransform _isdragging;
		ViewTransform _dragbtndown;
		int _dragx,_dragy;

		//! Previous pointer location
		paintcore::Point _prevpoint;
		paintcore::Point _prevoutlinepoint;
		float _pointerdistance;
		float _pointervelocity;
		StrokeSmoother _smoother;

		qreal _gestureStartZoom;
		qreal _gestureStartAngle;

		float _outlinesize;
		bool _enableoutline, _showoutline, _subpixeloutline;
		QCursor _cursor, _colorpickcursor;

		//! View zoom in percents
		qreal _zoom;
		//! View rotation in degrees
		qreal _rotate;

		drawingboard::CanvasScene *_scene;
		
		tools::ToolCollection _toolbox;
		tools::Tool *_current_tool;

		// Input settings
		int _smoothing;
		PressureMode _pressuremode;
		float _modeparam;
		KisCubicCurve _pressurecurve;
		KisCubicCurve _pressuredistance;
		KisCubicCurve _pressurevelocity;

		int _zoomWheelDelta;

		bool _locked;
		bool _pointertracking;
		bool _enableTabletEvents;
};

}

#endif

