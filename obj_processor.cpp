#include "obj_processor.h"
#include <chrono>
#include <QString>

OBJ_Processor::OBJ_Processor(QWidget *parent, QLabel * draw_space) : QWidget(parent) {
    this->draw_space = draw_space;
    draw_image  = new QImage(800,600, QImage::Format_RGB32);

    fill_stack = static_cast<vertex*>(malloc(image_size * sizeof(vertex)));
    zbuffer    = static_cast<double*>(malloc(image_size * sizeof(double)));

    background_color = qRgb(255,255,255);

    vertexes        = static_cast<vertex*>(malloc(sizeof(vertex)));
    screen_vertexes = static_cast<vertex*>(malloc(sizeof(vertex)));
    shadow_vertexes = static_cast<vertex*>(malloc(sizeof(vertex)));

    indicies        = static_cast<int   *>(malloc(sizeof(int)));

    objects         = static_cast<obj   *>(malloc(sizeof(obj)));

    setGeometry(0,0,800,600);
}

OBJ_Processor::~OBJ_Processor() {

    free(fill_stack);
    free(zbuffer);
    free(vertexes);
    free(screen_vertexes);
    free(shadow_vertexes);
    free(indicies);
    free(objects);

    delete draw_image;
}

void OBJ_Processor::add_object(vertex *vertexes, int *indicies,int vcount, int icount, QRgb color, vertex opos, vertex orot, vertex oscale, bool clean) {
    this->vertexes        = static_cast<vertex*>(realloc( this->vertexes,        (vertex_count+vcount) * sizeof(vertex) ) );
    this->screen_vertexes = static_cast<vertex*>(realloc( this->screen_vertexes, (vertex_count+vcount) * sizeof(vertex) ) );
    this->shadow_vertexes = static_cast<vertex*>(realloc( this->shadow_vertexes, (vertex_count+vcount) * sizeof(vertex) ) );

    this->indicies        = static_cast<int   *>(realloc( this->indicies,        (index_count+icount)  * sizeof(int ) ) );

    this->objects         = static_cast<obj   *>(realloc( this->objects,         (obj_count+1)         * sizeof(obj ) ) );


    objects[obj_count].color        = color;
    objects[obj_count].index_start  = index_count;
    objects[obj_count].index_end    = index_count + icount;
    objects[obj_count].vertex_start = vertex_count;
    objects[obj_count].vertex_end   = vertex_count + vcount;
    objects[obj_count].rot          = orot;
    objects[obj_count].scale        = oscale;
    objects[obj_count].pos          = opos;

    std::memcpy(this->vertexes+vertex_count,vertexes,vcount*sizeof(vertex));

    int pos = 0;
    for (int index = index_count; index < index_count + icount; ++index, pos++) {
        this->indicies[index] = indicies[pos] + vertex_count;
    }

    vertex_count += vcount;
    index_count  += icount;
    obj_count++;

    if (clean) {
        free(vertexes);
        free(indicies);
    }
}

void OBJ_Processor::rotate(int id, float x, float y, float z) {
    objects[id].rot.x += x;

    objects[id].rot.y += y;

    objects[id].rot.z += z;

}

void OBJ_Processor::set_light(const vertex & light) {
    this->light = light;

}

double OBJ_Processor::interpolate_z(const double & x, const double & y,const vertex & p1screen,const vertex & p2screen, const vertex & p3screen, const double & bcoefftopl1, const double & bcoefftopr1, const double & bcoefftopl2, const double & bcoefftopr2, const double & bcoeffbott) {
    double l1 = ( bcoefftopl1 * (x - p3screen.x) + bcoefftopr1 * (y - p3screen.y) ) / bcoeffbott;
    double l2 = ( bcoefftopl2 * (x - p3screen.x) + bcoefftopr2 * (y - p3screen.y) ) / bcoeffbott;

    return p1screen.z * l1 + p2screen.z * l2 + p3screen.z * (1 - l1 - l2);
}

bool OBJ_Processor::check_zbuffer(const int &x, const int &y, const double & z) {
    return zbuffer[x + y * 800] > z;
}


float triangle_dot(const vertex & p1, const vertex & p2, const vertex & p3) {
    return (static_cast<int>(p1.x) - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (static_cast<int>(p1.y) - p3.y);
}

// Только для по против часовой стрелки
bool point_inside_triangle(const vertex & p, const vertex & p1, const vertex & p2, const vertex & p3) {
    if ( triangle_dot(p, p1, p2) > 0.0005f) return false;
    if ( triangle_dot(p, p2, p3) > 0.0005f) return false;
    if ( triangle_dot(p, p3, p1) > 0.0005f) return false;
    return true;
}

// алгоритм быстрой отрисовки линии с потдержкой интерполяции Z координаты
void OBJ_Processor::efla_withz(const vertex &p1, const vertex &p2, const QRgb & color) {
    QRgb * image_line;

    int x = p1.x;
    int y = p1.y;

    double  z_inc;
    double  z = p1.z;

    bool y_longer  = false;
    int  short_len = (int)p2.y - y;
    int  long_len  = (int)p2.x - x;

    z_inc = ( p2.z - z ) / (p2.x - p1.x);

    if ( std::abs(short_len) > std::abs(long_len) ) {
        std::swap(short_len,long_len);
        y_longer = true;

        z_inc = ( p2.z - z ) / (p2.y - p1.y);
    }

    int dec_inc = 0;

    if (long_len) dec_inc = (short_len << 16) / long_len;

    if ( y_longer ) {
        if ( long_len > 0 ) {
            long_len += y;
            for (int  j = 0x8000 + ( x << 16); y <= long_len; ++y) {
                if ( y >= 0 && y < 600 && (j>>16) >=0 && (j>>16) < 800 && check_zbuffer(j >> 16, y, z)) {
                    image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(y));

                    image_line[j >> 16] = color;
                    zbuffer[(j >> 16) + y * 800] = z;
                }
                j += dec_inc;
                z += z_inc;
            }
            return;
        }
        long_len += y;
        for (int j = 0x8000 + ( x << 16 ); y >= long_len; --y) {
            if ( y >= 0 && y < 600 && ( j >> 16 ) >=0 && ( j >> 16 ) < 800 && check_zbuffer(j >> 16, y, z)) {
                image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(y));

                image_line[ j >> 16 ] = color;
                zbuffer[( j >> 16 ) + y * 800] = z;
            }
            j -= dec_inc;
            z -= z_inc;
        }
        return;
    }

    if ( long_len > 0 ) {
        long_len += x;
        for (int j = 0x8000 + ( y << 16 ); x <= long_len; ++x) {
            if (x >= 0 && x < 800 && (j >> 16) >=0 && ( j >> 16) < 600 && check_zbuffer(x, j >> 16, z)) {
                image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(j >> 16));

                image_line[x] = color;
                zbuffer[x + ( j >> 16 ) * 800] = z;
            }
            j += dec_inc;
            z += z_inc;
        }
        return;
    }
    long_len += x;
    for (int j = 0x8000 + ( y << 16 ); x >= long_len; --x) {
        if (x >= 0 && x < 800 && ( j>> 16 ) >=0 && ( j >> 16 ) < 600 && check_zbuffer(x, j >> 16, z)) {
            image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(j >> 16));

            image_line[x] = color;
            zbuffer[x + (j >> 16) * 800] = z;
        }
        j -= dec_inc;
        z -= z_inc;
    }
}

void OBJ_Processor::draw_filled_triangle(const int & vertex_id, const QRgb &edge_color) {
    int xl,xr,y,i;
    double inverseZ, xl_inverseZ, xr_inverseZ;
    bool left,right,left_offscreen, right_offscreen;
    int stack_offset = 0;

    bool move_left, move_right;

    QRgb * image_line;
    const QRgb * image_line_up;
    const QRgb * image_line_down;

    // получаем точки треугольника (экранные и оригинальные) по индексу

    vertex p1screen = screen_vertexes[indicies[vertex_id  ]];
    vertex p2screen = screen_vertexes[indicies[vertex_id+1]];
    vertex p3screen = screen_vertexes[indicies[vertex_id+2]];

    // отбрасываем треугольники за экраном и если смотрим на обратную сторону

    if (p1screen.z < 0 || p2screen.z < 0 || p3screen.z < 0) return;
    if ( ( (p2screen.x - p1screen.x) * (p3screen.y - p1screen.y) - (p2screen.y - p1screen.y) * (p3screen.x - p1screen.x) >= 0) ) return;

    // быстрый алгоритм линии, нужен для того, чтобы у краев тоже
    // была Z координата

    efla_withz(p1screen,p2screen,edge_color);
    efla_withz(p2screen,p3screen,edge_color);
    efla_withz(p3screen,p1screen,edge_color);

    // для данной отрисовки нужна интерполяция Z координаты
    // делается это через барицентрические координаты
    // их расчет можно ускорить, заранее рассчитав значения,
    // изменяющиеся только при смене треугольника

    double barycentric_coeff_bottom = (p2screen.y - p3screen.y) * (p1screen.x - p3screen.x) + (p3screen.x - p2screen.x) * (p1screen.y - p3screen.y);
    double barycentric_coeff_l1_top_left = (p2screen.y - p3screen.y);
    double barycentric_coeff_l2_top_left = (p3screen.y - p1screen.y);
    double barycentric_coeff_l1_top_right = (p3screen.x - p2screen.x);
    double barycentric_coeff_l2_top_right = (p1screen.x - p3screen.x);

    // стартовая точка для отрисовки, в идеале это место заменить бы
    vertex start_point = { (p1screen.x + p2screen.x + p3screen.x)/3, (p1screen.y + p2screen.y + p3screen.y)/3, 0 };

    // точка, находящаяся рядом с вершиной внутри треугольника
    vertex edge_close_point;


    if (start_point.y > 0 && start_point.y < 599 && start_point.x > 0 && start_point.x < 799) {
        if (point_inside_triangle(start_point,p1screen,p2screen,p3screen)) {
            inverseZ = interpolate_z(start_point.x,start_point.y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);

            if ( check_zbuffer(static_cast<int>(start_point.x),static_cast<int>(start_point.y),inverseZ) ) {
                if (reinterpret_cast<QRgb*>(draw_image->scanLine(start_point.y))[static_cast<int>(start_point.x)] != edge_color) {
                    fill_stack[stack_offset].x = start_point.x;
                    fill_stack[stack_offset].y = start_point.y;
                    stack_offset++;
                }
             }
        }
    }

    edge_close_point = {p1screen.x - (p1screen.x - start_point.x) * 0.15f, p1screen.y - (p1screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            inverseZ = interpolate_z(edge_close_point.x,edge_close_point.y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);

            if ( check_zbuffer(static_cast<int>(edge_close_point.x),static_cast<int>(edge_close_point.y),inverseZ) ) {
                if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                    fill_stack[stack_offset].x = edge_close_point.x;
                    fill_stack[stack_offset].y = edge_close_point.y;
                    stack_offset++;
                }
             }
         }
    }

    edge_close_point = {p2screen.x - (p2screen.x - start_point.x) * 0.15f, p2screen.y - (p2screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            inverseZ = interpolate_z(edge_close_point.x,edge_close_point.y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);

            if ( check_zbuffer(static_cast<int>(edge_close_point.x),static_cast<int>(edge_close_point.y),inverseZ) ) {
                if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                    fill_stack[stack_offset].x = edge_close_point.x;
                    fill_stack[stack_offset].y = edge_close_point.y;
                    stack_offset++;
                }
             }
        }
    }

    edge_close_point = {p3screen.x - (p3screen.x - start_point.x) * 0.15f, p3screen.y - (p3screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            inverseZ = interpolate_z(edge_close_point.x,edge_close_point.y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);

            if ( check_zbuffer(static_cast<int>(edge_close_point.x),static_cast<int>(edge_close_point.y),inverseZ) ) {
                if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                    fill_stack[stack_offset].x = edge_close_point.x;
                    fill_stack[stack_offset].y = edge_close_point.y;
                    stack_offset++;
                }
             }
        }
    }

    while(stack_offset) {

        y = fill_stack[stack_offset-1].y;

        if (y <= 0 || y >=599) {
            stack_offset--;
            break;
        }

        xl = fill_stack[stack_offset-1].x;
        xr = xl;
        stack_offset--;

        left = true;
        right = true;

        left_offscreen = false;
        right_offscreen = false;

        move_left = false;
        move_right = false;

        // в данном случае получаются строки - текущая, на которой будет происходить заливка и
        // верхняя / нижняя, которые проверяются на цвет для добавления в стек
        image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(y));
        image_line_up = reinterpret_cast<const QRgb*>(draw_image->constScanLine(y-1));
        image_line_down = reinterpret_cast<const QRgb*>(draw_image->constScanLine(y+1));

        // работает пока происходит движение влево или вправо
        while (true) {

            xl_inverseZ = interpolate_z(xl,y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
            xr_inverseZ = interpolate_z(xr,y,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);

            if ( xl<=0 || xl >=799 ) {left_offscreen   = true; left  = false; } else  if (image_line[xl] == edge_color || !check_zbuffer(xl,y,xl_inverseZ)) left = false;
            if ( xr<=0 || xr >=799 ) {right_offscreen  = true; right = false; } else  if (image_line[xr] == edge_color || !check_zbuffer(xr,y,xr_inverseZ)) right = false;

            if (left_offscreen && right_offscreen) break;

            if ( ! ( left || right ) ) {

                if (!left_offscreen) {
                    if (move_left) i = 1; else i = 0;

                    if (image_line_up[xl+i] != edge_color) {
                        inverseZ = interpolate_z(xl+i,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                        if ( check_zbuffer(xl+i,y-1,inverseZ) ) {
                                fill_stack[stack_offset].x = xl+i;
                                fill_stack[stack_offset].y = y-1;
                                stack_offset++;
                        }
                    }
                    if (image_line_down[xl+i] != edge_color) {
                        inverseZ = interpolate_z(xl+i,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                        if ( check_zbuffer(xl+i,y+1,inverseZ) ) {
                                fill_stack[stack_offset].x = xl+i;
                                fill_stack[stack_offset].y = y+1;
                                stack_offset++;
                        }
                    }
                }

                if (!right_offscreen) {
                    if (move_right) i = -1; else i = 0;

                    if (image_line_up[xr+i] != edge_color) {
                        inverseZ = interpolate_z(xr+i,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                        if (check_zbuffer(xr+i,y-1,inverseZ)) {
                                fill_stack[stack_offset].x = xr+i;
                                fill_stack[stack_offset].y = y-1;
                                stack_offset++;
                        }
                    }

                    if (image_line_down[xr+i] != edge_color) {
                        inverseZ = interpolate_z(xr+i,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                        if (check_zbuffer(xr+i,y+1,inverseZ)) {
                                fill_stack[stack_offset].x = xr+i;
                                fill_stack[stack_offset].y = y+1;
                                stack_offset++;
                        }
                    }
                }

                break;
            }

            if (left) {
                if ( check_zbuffer(xl,y,xl_inverseZ) && image_line[xl] != edge_color ) {
                    zbuffer[xl + 800 * y ] = xl_inverseZ;
                    image_line[xl] = edge_color;
                }

                inverseZ = interpolate_z(xl,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                if (image_line_up[xl] != edge_color && check_zbuffer(xl,y-1,inverseZ) ) {
                    inverseZ = interpolate_z(xl-1,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                    if (image_line_up[xl-1] == edge_color || !check_zbuffer(xl-1,y-1,inverseZ)) {
                        fill_stack[stack_offset].x = xl;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }
                }

                inverseZ = interpolate_z(xl,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                if (image_line_down[xl] != edge_color && check_zbuffer(xl,y+1,inverseZ) ) {
                    inverseZ = interpolate_z(xl-1,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                    if (image_line_down[xl-1] == edge_color || !check_zbuffer(xl-1,y+1,inverseZ)) {
                        fill_stack[stack_offset].x = xl;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                xl--;
                move_left = true;
            }

            if (right) {

                if ( check_zbuffer(xr,y,xr_inverseZ) && image_line[xr] != edge_color ) {
                    zbuffer[xr + 800 * y ] = xr_inverseZ;
                    image_line[xr] = edge_color;
                }

                inverseZ = interpolate_z(xr,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                if (image_line_up[xr] != edge_color && check_zbuffer(xr,y-1,inverseZ) ) {
                    inverseZ = interpolate_z(xr+1,y-1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                    if (image_line_up[xr+1] == edge_color || !check_zbuffer(xr+1,y-1,inverseZ)) {
                        fill_stack[stack_offset].x = xr;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }
                }

                inverseZ = interpolate_z(xr,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                if (image_line_down[xr] != edge_color && check_zbuffer(xr,y+1,inverseZ) ) {
                    inverseZ = interpolate_z(xr+1,y+1,p1screen,p2screen,p3screen,barycentric_coeff_l1_top_left,barycentric_coeff_l1_top_right,barycentric_coeff_l2_top_left,barycentric_coeff_l2_top_right,barycentric_coeff_bottom);
                    if (image_line_down[xr+1] == edge_color || !check_zbuffer(xr+1,y+1,inverseZ)) {
                        fill_stack[stack_offset].x = xr;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                xr++;
                move_right = true;
            }
        }
    }
}

// используется только для отрисовки теней
void OBJ_Processor::draw_filled_triangle_nonz(QPainter *painter, const int &vertex_id, const QRgb &edge_color) {
    int xl,xr,y,i;
    bool left,right,left_offscreen, right_offscreen;
    int stack_offset = 0;

    bool move_left, move_right;

    QRgb * image_line;
    const QRgb * image_line_up;
    const QRgb * image_line_down;

    // получаем точки треугольника (экранные и оригинальные) по индексу

    vertex p1screen = shadow_vertexes[indicies[vertex_id  ]];
    vertex p2screen = shadow_vertexes[indicies[vertex_id+1]];
    vertex p3screen = shadow_vertexes[indicies[vertex_id+2]];

    // отбрасываем треугольники за экраном и если смотрим на обратную сторону

    if (p1screen.z < 0 || p2screen.z < 0 || p3screen.z < 0) return;

    painter->save();
    painter->setPen(edge_color);
    painter->drawLine(p1screen.x,p1screen.y,p2screen.x,p2screen.y);
    painter->drawLine(p2screen.x,p2screen.y,p3screen.x,p3screen.y);
    painter->drawLine(p3screen.x,p3screen.y,p1screen.x,p1screen.y);
    painter->restore();

    // стартовая точка для отрисовки
    vertex start_point = { (p1screen.x + p2screen.x + p3screen.x)/3, (p1screen.y + p2screen.y + p3screen.y)/3, 0 };

    // точка, находящаяся рядом с вершиной внутри треугольника
    vertex edge_close_point;

    if (start_point.y > 0 && start_point.y < 599 && start_point.x > 0 && start_point.x < 799) {
        if (point_inside_triangle(start_point,p1screen,p2screen,p3screen)) {
            if (reinterpret_cast<QRgb*>(draw_image->scanLine(start_point.y))[static_cast<int>(start_point.x)] != edge_color) {
                fill_stack[stack_offset].x = start_point.x;
                fill_stack[stack_offset].y = start_point.y;
                stack_offset++;
            }
        }
    }

    edge_close_point = {p1screen.x - (p1screen.x - start_point.x) * 0.15f, p1screen.y - (p1screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                fill_stack[stack_offset].x = edge_close_point.x;
                fill_stack[stack_offset].y = edge_close_point.y;
                stack_offset++;
            }
        }
    }

    edge_close_point = {p2screen.x - (p2screen.x - start_point.x) * 0.15f, p2screen.y - (p2screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                fill_stack[stack_offset].x = edge_close_point.x;
                fill_stack[stack_offset].y = edge_close_point.y;
                stack_offset++;
            }
        }
    }

    edge_close_point = {p3screen.x - (p3screen.x - start_point.x) * 0.15f, p3screen.y - (p3screen.y - start_point.y) * 0.15f,0};

    if (edge_close_point.y > 0 && edge_close_point.y < 599 && edge_close_point.x > 0 && edge_close_point.x < 799) {
        if (point_inside_triangle(edge_close_point,p1screen,p2screen,p3screen)) {
            if (reinterpret_cast<QRgb*>(draw_image->scanLine(edge_close_point.y))[static_cast<int>(edge_close_point.x)] != edge_color) {
                fill_stack[stack_offset].x = edge_close_point.x;
                fill_stack[stack_offset].y = edge_close_point.y;
                stack_offset++;
            }
        }
    }

    while(stack_offset) {

        y = fill_stack[stack_offset-1].y;

        if (y <= 0 || y >=599) {
            stack_offset--;
            break;
        }

        xl = fill_stack[stack_offset-1].x;
        xr = xl;
        stack_offset--;

        left = true;
        right = true;

        left_offscreen = false;
        right_offscreen = false;

        move_left = false;
        move_right = false;

        // в данном случае получаются строки - текущая, на которой будет происходить заливка и
        // верхняя / нижняя, которые проверяются на цвет для добавления в стек
        image_line = reinterpret_cast<QRgb*>(draw_image->scanLine(y));
        image_line_up = reinterpret_cast<const QRgb*>(draw_image->constScanLine(y-1));
        image_line_down = reinterpret_cast<const QRgb*>(draw_image->constScanLine(y+1));

        // работает пока происходит движение влево или вправо
        while (true) {
            if ( xl<=0 || xl >=799 ) {left_offscreen   = true; left  = false; } else  if (image_line[xl] == edge_color) left = false;
            if ( xr<=0 || xr >=799 ) {right_offscreen  = true; right = false; } else  if (image_line[xr] == edge_color) right = false;

            if (left_offscreen && right_offscreen) break;

            if ( ! ( left || right ) ) {

                if (!left_offscreen) {
                    if (move_left) i = 1; else i = 0;

                    if (image_line_up[xl+i] != edge_color) {
                        fill_stack[stack_offset].x = xl+i;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }
                    if (image_line_down[xl+i] != edge_color) {
                        fill_stack[stack_offset].x = xl+i;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                if (!right_offscreen) {
                    if (move_right) i = -1; else i = 0;

                    if (image_line_up[xr+i] != edge_color) {
                        fill_stack[stack_offset].x = xr+i;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }

                    if (image_line_down[xr+i] != edge_color) {
                        fill_stack[stack_offset].x = xr+i;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                break;
            }

            if (left) {
                if ( image_line[xl] != edge_color ) image_line[xl] = edge_color;

                if (image_line_up[xl] != edge_color) {
                    if (image_line_up[xl-1] == edge_color) {
                        fill_stack[stack_offset].x = xl;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }
                }
                if (image_line_down[xl] != edge_color) {
                    if (image_line_down[xl-1] == edge_color) {
                        fill_stack[stack_offset].x = xl;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                xl--;
                move_left = true;
            }

            if (right) {

                if ( image_line[xr] != edge_color ) image_line[xr] = edge_color;

                if (image_line_up[xr] != edge_color) {
                    if (image_line_up[xr+1] == edge_color) {
                        fill_stack[stack_offset].x = xr;
                        fill_stack[stack_offset].y = y-1;
                        stack_offset++;
                    }
                }

                if (image_line_down[xr] != edge_color) {
                    if (image_line_down[xr+1] == edge_color) {
                        fill_stack[stack_offset].x = xr;
                        fill_stack[stack_offset].y = y+1;
                        stack_offset++;
                    }
                }

                xr++;
                move_right = true;
            }
        }
    }

    QPainterPath path;
    path.moveTo(p1screen.x,p1screen.y);
    path.lineTo(p2screen.x,p2screen.y);
    path.lineTo(p3screen.x,p3screen.y);
    path.lineTo(p1screen.x,p1screen.y);
    painter->fillPath(path,QBrush(edge_color));

}

void OBJ_Processor::calculate_vertex_sreen_projection() {

    float x,y,z,f;
    float sin_x;        float cos_x;
    float sin_y;        float cos_y;
    float sin_z;        float cos_z;
    float sin_cam_y;    float cos_cam_y;
    float sin_cam_x;    float cos_cam_x;

    sin_cam_x = sin(rot.x);    cos_cam_x = cos(rot.x);
    sin_cam_y = sin(rot.y);    cos_cam_y = cos(rot.y);

    // вращение точки освещения для отображения её на экране

    screen_light = light;

    screen_light -= pos;

    // вращение по камере вокруг оси XZ
    x = screen_light.x;
    screen_light.x = x * cos_cam_y - screen_light.z * sin_cam_y;
    screen_light.z = x * sin_cam_y + screen_light.z * cos_cam_y;

    // вращение по камере вокруг оси YZ
    y = screen_light.y;
    screen_light.y = y * cos_cam_x - screen_light.z * sin_cam_x;
    screen_light.z = y * sin_cam_x + screen_light.z * cos_cam_x;

    if (screen_light.z >= 0) {

        // для преобразования к экрану координаты делятся на Z и умножаются на длину самой длинной стороны
        f = 800 / (screen_light.z);

        screen_light.x*=f;
        screen_light.y*=f;

        // после чего прибавляются половины сторон, чтобы вращение камеры шло от середины
        screen_light.x += 400;
        screen_light.y += 300;
    }

    // тоже самое но для точек объектов
    obj calc_obj;
    for (int objid = 0; objid < obj_count; ++objid) {
        calc_obj = objects[objid];

        // вычисляются синусы и косинусы для всех точек объекта
        sin_x = sin(calc_obj.rot.x);             cos_x = cos(calc_obj.rot.x);
        sin_y = sin(calc_obj.rot.y);             cos_y = cos(calc_obj.rot.y);
        sin_z = sin(calc_obj.rot.z);             cos_z = cos(calc_obj.rot.z);

        for (int vertexid = calc_obj.vertex_start; vertexid < calc_obj.vertex_end; ++vertexid) {

            // после чего точка вращается вокруг центра
            // по оси YZ
            screen_vertexes[vertexid].y = vertexes[vertexid].y * cos_x - vertexes[vertexid].z * sin_x;
            screen_vertexes[vertexid].z = vertexes[vertexid].y * sin_x + vertexes[vertexid].z * cos_x;

            // по оси XZ
            screen_vertexes[vertexid].x = vertexes[vertexid].x * cos_y - screen_vertexes[vertexid].z * sin_y;
            screen_vertexes[vertexid].z = vertexes[vertexid].x * sin_y + screen_vertexes[vertexid].z * cos_y;

            // по оси XY
            x = screen_vertexes[vertexid].x;
            screen_vertexes[vertexid].x = x * cos_z - screen_vertexes[vertexid].y * sin_z;
            screen_vertexes[vertexid].y = x * sin_z + screen_vertexes[vertexid].y * cos_z;

            // после чего изменяется положение в зависимости от размера и положения объекта
            screen_vertexes[vertexid] *= calc_obj.scale;
            screen_vertexes[vertexid] += calc_obj.pos;


            // из этих точек получаем дубликаты для тени
            shadow_vertexes[vertexid] = screen_vertexes[vertexid];

            x = shadow_vertexes[vertexid].x;
            y = shadow_vertexes[vertexid].y;
            z = shadow_vertexes[vertexid].z;

            f = (y - light.y);
            if (std::abs(f) <= 0.00001) f = 1;

            // преобразовываем точку на землю
            shadow_vertexes[vertexid].x = (x * (light_plane_y - light.y) - light.x * (light_plane_y - light.y)) / f;
            shadow_vertexes[vertexid].z = (z * (light_plane_y - light.y) - light.z * (light_plane_y - light.y)) / f;
            shadow_vertexes[vertexid].y = light_plane_y;


            // теперь вращаем точку относительно камеры
            screen_vertexes[vertexid] -= pos;

            shadow_vertexes[vertexid] -= pos;

            // вращаем в плоскости XZ
            x = screen_vertexes[vertexid].x;
            screen_vertexes[vertexid].x = x * cos_cam_y - screen_vertexes[vertexid].z * sin_cam_y;
            screen_vertexes[vertexid].z = x * sin_cam_y + screen_vertexes[vertexid].z * cos_cam_y;

            // вращаем в плоскости YZ
            y = screen_vertexes[vertexid].y;
            screen_vertexes[vertexid].y = y * cos_cam_x - screen_vertexes[vertexid].z * sin_cam_x;
            screen_vertexes[vertexid].z = y * sin_cam_x + screen_vertexes[vertexid].z * cos_cam_x;


            x = shadow_vertexes[vertexid].x;
            shadow_vertexes[vertexid].x = x * cos_cam_y - shadow_vertexes[vertexid].z * sin_cam_y;
            shadow_vertexes[vertexid].z = x * sin_cam_y + shadow_vertexes[vertexid].z * cos_cam_y;

            y = shadow_vertexes[vertexid].y;
            shadow_vertexes[vertexid].y = y * cos_cam_x - shadow_vertexes[vertexid].z * sin_cam_x;
            shadow_vertexes[vertexid].z = y * sin_cam_x + shadow_vertexes[vertexid].z * cos_cam_x;

            if (screen_vertexes[vertexid].z >= 0) {

                // и преобразовываем к экрану деля на Z
                f = 800 / (screen_vertexes[vertexid].z);

                screen_vertexes[vertexid].x*=f;
                screen_vertexes[vertexid].y*=f;

                screen_vertexes[vertexid].x += 400;
                screen_vertexes[vertexid].y += 300;

            }

            if (shadow_vertexes[vertexid].z >= 0) {

                f = 800 / (shadow_vertexes[vertexid].z);

                shadow_vertexes[vertexid].x*=f;
                shadow_vertexes[vertexid].y*=f;

                shadow_vertexes[vertexid].x += 400;
                shadow_vertexes[vertexid].y += 300;
            }
        }
    }
}

void OBJ_Processor::update_cam() {

    double x = sin(rot.y) * 0.05;
    double y = cos(rot.y) * 0.05;

    if (mouse_lock) {
        QPoint m_pos = mapFromGlobal(QCursor::pos());
        rot.x += (m_pos.y() - 600.f/2)/600;
        rot.y += (m_pos.x() - 800.f/2)/600;

        QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
    }

    if (w_state) {
        pos.x += x ;
        pos.z += y;
    }

    if (s_state) {
        pos.x -= x;
        pos.z -= y;
    }

    if (a_state) {
        pos.x -= y;
        pos.z += x;
    }

    if (d_state) {
        pos.x += y;
        pos.z -= x;
    }

    if (space_state) {
        pos.y -= 0.05;
    }

    if (ctrl_state) {
        pos.y += 0.05;
    }
}

void OBJ_Processor::paintEvent(QPaintEvent *event) {

    update_cam();

    draw_image->fill(Qt::white);
    memset(zbuffer,127,image_size * sizeof(double));
    QPainter painter(draw_image);
    painter.setPen(Qt::black);

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    obj calc_obj;

    if (draw) {

        calculate_vertex_sreen_projection();

        begin = std::chrono::steady_clock::now();

        for (int objid = 0; objid < obj_count; ++objid) {
            calc_obj = objects[objid];
            for (int indexid = calc_obj.index_start; indexid < calc_obj.index_end; indexid+=3) {
                draw_filled_triangle_nonz(&painter,indexid,qRgb(20,20,20));
            }
        }


        for (int objid = 0; objid < obj_count; ++objid) {
            calc_obj = objects[objid];
            for (int indexid = calc_obj.index_start; indexid < calc_obj.index_end; indexid+=3) {
                draw_filled_triangle(indexid,calc_obj.color);
            }
        }

        if (screen_light.z >= 0) painter.drawEllipse(screen_light.x,screen_light.y,20,20);


        end = std::chrono::steady_clock::now();

        painter.drawText(20,20,QString::number(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
        painter.drawText(20,40,QString::number(rot.x));
        painter.drawText(20,60,QString::number(rot.y));
    }


    draw_space->setPixmap(QPixmap::fromImage(*draw_image));
    update();
}
