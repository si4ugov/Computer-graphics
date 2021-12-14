#ifndef OBJ_PROCESSOR_H
#define OBJ_PROCESSOR_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <math3D.h>
#include <QColor>
#include <malloc.h>
#include <QRgb>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <QPainterPath>
#include <iostream>

class OBJ_Processor : public QWidget {
    Q_OBJECT
    public:
        explicit OBJ_Processor(QWidget *parent, QLabel * draw_space);
                ~OBJ_Processor() override;


        QRgb background_color;
        bool draw = false;

        bool w_state = false;
        bool s_state = false;
        bool a_state = false;
        bool d_state = false;
        bool space_state = false;
        bool ctrl_state = false;

        bool mouse_lock = false;

        // добавление объекта
        void add_object(vertex * vertexes, int * indicies,int vcount, int icount, QRgb color, vertex opos = {0,0,0}, vertex orot = {0,0,0}, vertex oscale = {1,1,1}, bool clean = false);

        // установка точки освещения
        void set_light(const vertex & light);

        // вращение объекта
        void rotate(int id, float x, float y, float z);

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:

        // отрисовка закрашенного треугольника без Z интерполяции
        void draw_filled_triangle_nonz(QPainter * painter, const int & vertex_id, const QRgb &edge_color);

        // отрисовка закрашенного треугольника с Z интерполяцией с помощью барицентрических коордиант
        void draw_filled_triangle(const int & vertex_id, const QRgb &edge_color);

        // Алгоритм для отрисовки линии взят Extremly Fast Line Algorithm var E by Po-Han Lin
        // чтобы не изобретать велосипед и не писать медлительный брезенхем
        volatile char link[34] = "http://www.edepot.com/lineex.html";
        void efla_withz(const vertex & p1, const vertex & p2, const QRgb & color);

        // интерполяция через барицентрические координаты
        double interpolate_z(const double & x, const double & y,const vertex & p1screen,const vertex & p2screen, const vertex & p3screen, const double & bcoefftopl1, const double & bcoefftopr1, const double & bcoefftopl2, const double & bcoefftopr2, const double & bcoeffbott);

        // проверка плоского Z буффера по координатам
        bool check_zbuffer(const int & x, const int & y, const double & z);

        // преобразование координат объектов к координатам мира и их последующее преобразование к экранным координатам
        void calculate_vertex_sreen_projection();

        // функция обновления позиции и вращения камеры
        void update_cam();

        vertex * fill_stack;
        double * zbuffer;
        unsigned int image_size = 800 * 600;

        QImage * draw_image;
        QLabel * draw_space;


        int vertex_count            = 0;
        int index_count             = 0;
        int obj_count               = 0;

        vertex * vertexes;
        vertex * screen_vertexes;
        vertex * shadow_vertexes;

        int * indicies;

        obj * objects;

        // lights

        vertex light;
        vertex screen_light;
        float light_plane_y = 5;

        // camera

        double far_plane = 20;
        vertex pos = {-5,0,-4};
        vertex rot = {0,1.1,0};

};

#endif // OBJ_PROCESSOR_H
