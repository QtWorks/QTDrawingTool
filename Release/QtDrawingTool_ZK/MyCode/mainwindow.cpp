#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDebug>
#include <QPoint>
#include "circle.h"
#include "line.h"
#include "rect.h"
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. ��ʼ�����еĲ˵�������������Group��Action
    initMenusToolBarsAndActionGroups();
    initAllActions();

    // 2. ���״̬��
    statusBar();

    // 3. �����������󣬾���
    paintWidget = new PaintWidget(this);
    setCentralWidget(paintWidget);

    // 4. ����
    connectAllThings();
}

// ��ʼ��Menu, ToolBar��QActionGroup
void MainWindow::initMenusToolBarsAndActionGroups()
{
    this->fileMenu = menuBar()->addMenu(tr("&File"));
    this->drawMenu = menuBar()->addMenu(tr("&Draw"));
    this->editMenu = menuBar()->addMenu(tr("&Edit"));

    this->fileToolBar = this->addToolBar(tr("&File"));
    this->drawToolBar = this->addToolBar(tr("&Draw"));
    this->editToolBar = this->addToolBar(tr("&Edit"));

    this->group = new QActionGroup(this);
}

// ��ʼ�����е�Action
void MainWindow::initAllActions()
{
    // ��Action
    this->openAction = new QAction(tr("&Open"), this);
    openAction->setToolTip(tr("Open a file."));
    openAction->setStatusTip(tr("Open a file."));
    fileMenu->addAction(openAction);
    fileToolBar->addAction(openAction);

    // ����Action
    this->saveAction = new QAction(tr("&Save"), this);
    saveAction->setToolTip(tr("Save this file."));
    saveAction->setStatusTip(tr("Save this file."));
    fileMenu->addAction(saveAction);
    fileToolBar->addAction(saveAction);

    // ��ӡAction
    this->printAction = new QAction(tr("&Print"), this);
    printAction->setToolTip(tr("Print to ( *.oxps  *.xps ) file."));
    printAction->setStatusTip(tr("Print to ( *.oxps  *.xps ) file."));
    fileMenu->addAction(printAction);
    fileToolBar->addAction(printAction);

    // ����Action
    this->drawLineAction = new QAction(tr("&Line"), this);
    drawLineAction->setToolTip(tr("Draw a line."));
    drawLineAction->setStatusTip(tr("Draw a line."));
    drawLineAction->setCheckable(true);
    drawLineAction->setChecked(true);
    group->addAction(drawLineAction);
    drawToolBar->addAction(drawLineAction);
    drawMenu->addAction(drawLineAction);

    // ������Action
    this->drawRectAction = new QAction(tr("&Rectangle"), this);
    drawRectAction->setToolTip(tr("Draw a rectangle."));
    drawRectAction->setStatusTip(tr("Draw a rectangle."));
    drawRectAction->setCheckable(true);
    group->addAction(drawRectAction);
    drawToolBar->addAction(drawRectAction);
    drawMenu->addAction(drawRectAction);

    // ��ԲAction
    this->drawCircleAction = new QAction(tr("&Circle"), this);
    drawCircleAction->setToolTip(tr("Draw a circle."));
    drawCircleAction->setStatusTip(tr("Draw a circle."));
    drawCircleAction->setCheckable(true);
    group->addAction(drawCircleAction);
    drawToolBar->addAction(drawCircleAction);
    drawMenu->addAction(drawCircleAction);

    // ����Action
    this->undoAction = new QAction(tr("&Undo"), this);
    undoAction->setToolTip(tr("Undo last shape."));
    undoAction->setStatusTip(tr("Undo last shape."));
    editMenu->addAction(undoAction);
    editToolBar->addAction(undoAction);

    // ���Action
    this->clearAllAction = new QAction(tr("&Clear All"), this);
    clearAllAction->setToolTip(tr("Clear all shapes."));
    clearAllAction->setStatusTip(tr("Clear all shapes."));
    editMenu->addAction(clearAllAction);
    editToolBar->addAction(clearAllAction);
}

// connect���еĺ���
void MainWindow::connectAllThings()
{
    // ��������File Action
    connect(saveAction, SIGNAL(triggered()),
            this, SLOT(save()));
    connect(openAction, SIGNAL(triggered()),
            this, SLOT(open()));
    connect(printAction, SIGNAL(triggered()),
            this, SLOT(print()));

    // ��������Draw Action
    connect(drawLineAction, SIGNAL(triggered()),
            this, SLOT(drawLineActionTriggered()));
    connect(drawRectAction, SIGNAL(triggered()),
            this, SLOT(drawRectActionTriggered()));
    connect(drawCircleAction, SIGNAL(triggered()),
            this, SLOT(drawCircleActionTriggered()));

    // ��������Edit Action
    connect(clearAllAction, SIGNAL(triggered()),
            paintWidget, SLOT(clearAll()));
    connect(undoAction, SIGNAL(triggered()),
            paintWidget, SLOT(undo()));

    // ���� MainWindow������changeCurrentShape�ź�
    // �� paintWidget��setCurrentShape��
    connect(this, SIGNAL(changeCurrentShape(Shape::Code)),
            paintWidget, SLOT(setCurrentShape(Shape::Code)));
}

void MainWindow::drawLineActionTriggered()
{
    emit changeCurrentShape(Shape::Line);
}

void MainWindow::drawRectActionTriggered()
{
    emit changeCurrentShape(Shape::Rect);
}

void MainWindow::drawCircleActionTriggered()
{
    emit changeCurrentShape(Shape::Circle);
}

/*
 *  QFileDialog Class Reference
    QString	getExistingDirectory ( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), Options options = ShowDirsOnly )
    QString	getOpenFileName ( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, Options options = 0 )
    QStringList	getOpenFileNames ( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, Options options = 0 )
    QString	getSaveFileName ( QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, Options options = 0 )
 */
void MainWindow::open()
{
    // 1. ��ȡ�ļ�·�������ļ����������ļ��Ƿ�򿪡�����ֱ��return
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    ".", tr("dat Files (*.dat)"));
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(NULL, tr("Warning"), tr("File not opened"));
        return;
    }

    // 2. ��������б�
    paintWidget->shapeList.clear();

    // 3. ����һ��QtextStream����������file
    QTextStream in(&file);

    // 4. ��QtextStream����û�е�ĩβʱ��ѭ��
    while (! in.atEnd())
    {
        // 4.1 ȡ���ļ��е�shapeName, ����s��
        QString s;
        in >> s;
//        qDebug() << s;
        if (s == "")
            break;

        // 4.2 �����ļ���shapeName��ֵ����������Shape����һ���Ӷ���Ȼ���newShapeָ���´����Ķ���
        Shape * newShape;
        if (s == "Circle")
            newShape = new Circle(this->paintWidget);
        else if (s == "Rect")
            newShape = new Rect(this->paintWidget);
        else if (s == "Line")
            newShape = new Line(this->paintWidget);

        // 4.3 ��ȡ�ļ��е��ĸ�����ֵ�������´�����Shape���Ӷ�����
        int sx, sy, ex, ey;
        in >> sx >> sy >> ex >> ey;
//        qDebug() << sx << sy << ex << ey;
        newShape->setStart(QPoint(sx, sy));
        newShape->setEnd(QPoint(ex, ey));

        // 4.4 �Ѵ����õ�Shape���Ӷ�����ӵ������б���
        paintWidget->shapeList.append(newShape);
    }

    // 5. ˢ�»�������ʾ����һ����ӵ����������ݡ��ر��ļ�
    paintWidget->update();
    file.close();
}

void MainWindow::save()
{
    // 1. ��ȡ�ļ�·�������ļ����������ļ��Ƿ�򿪡�����ֱ��return
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    ".", tr("dat Files (*.dat)"));
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(NULL, tr("Warning"), tr("File not saved!"));
        return;
    }

    // 2. ����һ��QtextStream����������file
    QTextStream out(&file);

    // 3. ͨ��ѭ�����������б��е�ÿһ��Shape����ĸ������Զ�д���ļ��С��ر��ļ�
    foreach (Shape *shape, paintWidget->shapeList)
    {
        // windows�»�����\r\n��unix�»�����\n���������ĩֻ���\n, ��windows���ü��±��򿪴��ļ�ʱ�ᶪʧ���з�����������ĩ���\r\n
        out << shape->shapeName         << "\r\n"
            << shape->getStart().x()    << "\r\n"
            << shape->getStart().y()    << "\r\n"
            << shape->getEnd().x()      << "\r\n"
            << shape->getEnd().y()      << "\r\n";
    }
    file.close();
}

void MainWindow::print()
{
    QPrinter printer;
    QPainter painter;
    painter.begin(&printer);
    double xscale = printer.pageRect().width()/double(this->paintWidget->width());
    double yscale = printer.pageRect().height()/double(this->paintWidget->height());
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                      printer.paperRect().y() + printer.pageRect().height()/2);
    painter.scale(scale, scale);
    painter.translate(-width()/2, -height()/2);

    this->paintWidget->render(&painter);
}
