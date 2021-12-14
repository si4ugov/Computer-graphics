#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    objproc = new OBJ_Processor(ui->l_drawspace,ui->l_drawspace);

    vertex paralelepiped[8] = {
        {  1.000000, -1.000000,  2.000000   },
        {  1.000000,  1.000000,  2.000000   },
        {  1.000000, -1.000000, -2.000000   },
        {  1.000000,  1.000000, -2.000000   },
        { -1.000000, -1.000000,  2.000000   },
        { -1.000000,  1.000000,  2.000000   },
        { -1.000000, -1.000000, -2.000000   },
        { -1.000000,  1.000000, -2.000000   }
    };

    int paralelepiped_indicies[36] = {
        5, 3, 1,
        3, 8, 4,
        7, 6, 8,
        2, 8, 6,
        1, 4, 2,
        5, 2, 6,
        5, 7, 3,
        3, 7, 8,
        7, 5, 6,
        2, 4, 8,
        1, 3, 4,
        5, 1, 2
    };

    for (int i = 0; i < 36; ++i) {
        paralelepiped_indicies[i]--;
    }


    vertex piramyd[4] = {
        {  0.000000,  1.000000,  2.000000 },
        {  1.732051,  1.000000, -1.000000 },
        { -1.732051,  1.000000, -1.000000 },
        {  0.000000, -1.000000,  0.000000 }
    };

    int piramyd_indicies[12] = {
        1, 4, 2,
        1, 2, 3,
        2, 4, 3,
        3, 4, 1
    };

    for (int i = 0; i < 12; ++i) {
        piramyd_indicies[i]--;
    }


    objproc->set_light({-4,-4,-4});

    objproc->add_object(paralelepiped,paralelepiped_indicies,8,36,qRgb(255,0,0),{2,0,0});
    objproc->add_object(piramyd,piramyd_indicies,4,12,qRgb(255,155,0),{0,-0.9,0},{0.14,-0.5,0});

}

MainWindow::~MainWindow() {

    delete objproc;

    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if( event->key() == Qt::Key_R ) {
        objproc->draw = !objproc->draw;
    }
    if( event->key() == Qt::Key_T ) {
        objproc->mouse_lock = !objproc->mouse_lock;
    }

    if( event->key() == Qt::Key_W ) {
        objproc->w_state = true;
    }
    if( event->key() == Qt::Key_S ) {
        objproc->s_state = true;
    }
    if( event->key() == Qt::Key_A ) {
        objproc->a_state = true;
    }
    if( event->key() == Qt::Key_D ) {
        objproc->d_state = true;
    }
    if ( event->key() == Qt::Key_E) {
        objproc->ctrl_state = true;
    }
    if ( event->key() == Qt::Key_Q) {
        objproc->space_state = true;
    }

    // вращение паралелепипеда + -
    if( event->key() == Qt::Key_P ) {
        objproc->rotate(0,0.05,0,0);
    }
    if( event->key() == Qt::Key_BracketLeft ) {
        objproc->rotate(0,0,0.05,0);
    }
    if( event->key() == Qt::Key_BracketRight ) {
        objproc->rotate(0,0,0,0.05);
    }

    if( event->key() == Qt::Key_L ) {
        objproc->rotate(0,-0.05,0,0);
    }
    if( event->key() == Qt::Key_Semicolon) {
        objproc->rotate(0,0,-0.05,0);
    }
    if( event->key() == Qt::Key_Apostrophe ) {
        objproc->rotate(0,0,0,-0.05);
    }


    // вращение пирамиды + -
    if( event->key() == Qt::Key_U ) {
        objproc->rotate(1,0.05,0,0);
    }
    if( event->key() == Qt::Key_I ) {
        objproc->rotate(1,0,0.05,0);
    }
    if( event->key() == Qt::Key_O ) {
        objproc->rotate(1,0,0,0.05);
    }

    if( event->key() == Qt::Key_H ) {
        objproc->rotate(1,-0.05,0,0);
    }
    if( event->key() == Qt::Key_J ) {
        objproc->rotate(1,0,-0.05,0);
    }
    if( event->key() == Qt::Key_K ) {
        objproc->rotate(1,0,0,-0.05);
    }


}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if( event->key() == Qt::Key_W ) {
        objproc->w_state = false;
    }
    if( event->key() == Qt::Key_S ) {
        objproc->s_state = false;
    }
    if( event->key() == Qt::Key_A ) {
        objproc->a_state = false;
    }
    if( event->key() == Qt::Key_D ) {
        objproc->d_state = false;
    }
    if ( event->key() == Qt::Key_E) {
        objproc->ctrl_state = false;
    }
    if ( event->key() == Qt::Key_Q) {
        objproc->space_state = false;
    }
}
