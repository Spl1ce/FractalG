#include <SFML/Graphics.hpp>
#include <gmp.h>

const unsigned int w = 640, h = 480;
const unsigned int p = 1 << 8;
unsigned int bailPow = 2;
double scale = 1;

static mpf_t x, y, zoom, bail;

static sf::RenderWindow app(sf::VideoMode(w, h), "Mandelbrot Set");

const unsigned int colors = 90;
const unsigned char r[colors] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
const unsigned char g[colors] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char b[colors] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11};

struct pixel
{
    unsigned int iteration;
    bool done; mpf_t cr, ci, zr, zi;
    pixel()
    {
        iteration = 0; done = false;
        mpf_init2(cr,p); mpf_init2(ci,p);
        mpf_init2(zr,p); mpf_init2(zi,p);
    }
};

static const unsigned int pixelC = w * h;
static unsigned char pixels[pixelC << 2];
static pixel nums[pixelC];
static bool draw = false;

void resetNums()
{
    mpf_t ratio, xStart, tXIter, tYIter, xIter, yIter, py, px;
    mpf_init2(ratio, p); mpf_init2(xStart, p); mpf_init2(tXIter, p); mpf_init2(tYIter, p); mpf_init2(xIter, p); mpf_init2(yIter, p); mpf_init2(py, p); mpf_init2(px, p);
    mpf_set_d(ratio, ((double)app.getSize().x)/((double)app.getSize().y));
    mpf_sub(xStart, x, zoom);
    mpf_mul_ui(xIter, ratio, 2); mpf_mul(xIter, xIter, zoom); mpf_div_ui(xIter, xIter, w);
    mpf_mul_ui(yIter, zoom, 2); mpf_div_ui(yIter, yIter, h);
    mpf_div_ui(tXIter, xIter, 2); mpf_div_ui(tYIter, yIter, 2);
    mpf_add(xStart, xStart, tXIter); mpf_clear(tXIter);
    mpf_add(py,py,y); mpf_sub(py,py,zoom);
    mpf_add(py, py, tYIter); mpf_clear(tYIter);
    unsigned int i = 0;
    for(unsigned int iy = 0; iy < h; iy++)
    {
        mpf_set(px, xStart);
        for(unsigned int ix = 0; ix < w; ix++)
        {
            mpf_set(nums[i].cr, px); mpf_set(nums[i].ci, py);
            mpf_set(nums[i].zr, px); mpf_set(nums[i].zi, py);
            nums[i].iteration = 0; nums[i].done = false;
            pixels[i*4 + 0] = 0; pixels[i*4 + 1] = 0;
            pixels[i*4 + 2] = 0; pixels[i*4 + 3] = 64;
            mpf_add(px, px, xIter); i++;
        } mpf_add(py, py, yIter);
    }
    mpf_clear(ratio); mpf_clear(xStart); mpf_clear(xIter); mpf_clear(yIter); mpf_clear(py); mpf_clear(px);
}

/** MATH START **/
void iterate(pixel &temp)
{
    if(temp.done) { return; }
    mpf_t sqzr, sqzi, comp;
    mpf_init2(sqzr, p); mpf_init2(sqzi, p); mpf_init2(comp, p);
    mpf_mul(sqzr, temp.zr, temp.zr);    /// sqzr = zr*zr
    mpf_mul(sqzi, temp.zi, temp.zi);    /// sqzi = zi*zi
    mpf_add(comp, sqzr, sqzi); /// Adding squares for compairison
    if(mpf_cmp(comp, bail) > 0) { temp.done = true; return;}
    mpf_mul(temp.zi, temp.zr, temp.zi); /// zi = 2*zr*zi + ci
    mpf_add(temp.zi, temp.zi, temp.zi); /// |
    mpf_add(temp.zi, temp.zi, temp.ci); /// |
    mpf_sub(temp.zr, sqzr, sqzi);       /// zr = cr + sqzr - sqzi
    mpf_add(temp.zr, temp.zr, temp.cr); /// |
    temp.iteration++;
    mpf_clear(sqzr); mpf_clear(sqzi); mpf_clear(comp);
}

void drawFract(const unsigned int threadNum)
{
    const unsigned int max = (pixelC / 8) * (1 + threadNum);
    for(unsigned int pix = (pixelC / 8) * threadNum; pix < max; pix++)
    {
        iterate(nums[pix]); iterate(nums[pix]);
        iterate(nums[pix]); iterate(nums[pix]);
        const unsigned int counter = pix << 2;
        if(nums[pix].done)
        {
            if(!draw) { draw = true; }
            const unsigned long ii = nums[pix].iteration%colors;
            pixels[counter + 0] = r[ii];
            pixels[counter + 1] = g[ii];
            pixels[counter + 2] = b[ii];
            pixels[counter + 3] = 255;
        }
    }
}
/** MATH END **/

void reset(){ mpf_set_d(x,-1.5); mpf_set_d(y,0); mpf_set_d(zoom,2); }

#define drawPixels; texture.update(pixels); app.draw(sprite); app.display();
#define launchBots; bot0.launch(); bot1.launch(); bot2.launch(); bot3.launch(); bot4.launch(); bot5.launch(); bot6.launch(); bot7.launch();
#define waitBots; bot0.wait(); bot1.wait(); bot2.wait(); bot3.wait(); bot4.wait(); bot5.wait(); bot6.wait(); bot7.wait();
int main()
{
    mpf_init2(bail, p);
    mpf_set_ui(bail, (1 << bailPow));
    mpf_init2(x, p);
    mpf_init2(y, p);
    mpf_init2(zoom, p);
    sf::Texture texture;
    texture.create(w,h);
    texture.setSmooth(true);
    sf::Sprite sprite(texture);
    sf::Thread  bot0(drawFract,0), bot1(drawFract,1),
                bot2(drawFract,2), bot3(drawFract,3),
                bot4(drawFract,4), bot5(drawFract,5),
                bot6(drawFract,6), bot7(drawFract,7);
    reset();
    resetNums();
    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            /** Handels Mouse Stuff**/
            mpf_t modx, mody;
            mpf_t zmod;
            mpf_t tx, ty;
            mpf_t wRatio, hRatio, ratio;
            mpf_init2(modx, p); mpf_init2(mody, p);
            mpf_init2(zmod, p); mpf_init2(tx, p); mpf_init2(ty, p);
            mpf_init2(wRatio, p); mpf_init2(hRatio, p); mpf_init2(ratio, p);
            while(sf::Mouse::isButtonPressed(sf::Mouse::Left)){}
            mpf_set_ui(ratio, app.getSize().y); mpf_div_ui(ratio, ratio, app.getSize().x);
            mpf_set_ui(wRatio, w); mpf_div_ui(wRatio, wRatio, app.getSize().x);
            mpf_set_ui(hRatio, h); mpf_div_ui(hRatio, hRatio, app.getSize().y);
            mpf_set_ui(tx, sf::Mouse::getPosition(app).x);
            mpf_mul(tx,tx,wRatio);
            mpf_set_ui(ty, sf::Mouse::getPosition(app).y);
            mpf_mul(ty,ty,hRatio);
            mpf_set_ui(modx, w);
            mpf_div_ui(modx, modx, 2);
            mpf_mul(modx, modx, ratio);
            mpf_set_ui(mody, h);
            mpf_div_ui(mody, mody, 2);
            sprite.setOrigin(mpf_get_d(tx), mpf_get_d(ty));
            sprite.setPosition(mpf_get_d(tx), mpf_get_d(ty));
            mpf_sub(tx, tx, modx); mpf_sub(ty, ty, mody);
            mpf_mul(tx, tx, zoom); mpf_mul(ty, ty, zoom);
            mpf_div(tx, tx, modx); mpf_div(ty, ty, mody);
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            {
                scale *= 0.25;
                mpf_set_d(zmod, -3);
                mpf_mul(tx,tx,zmod);
                mpf_mul(ty,ty,zmod);
                mpf_add(x,x,tx);
                mpf_add(y,y,ty);
                mpf_mul_ui(zoom, zoom, 4);
            } else
            {
                scale *= 4;
                mpf_set_d(zmod, 3.f/4.f);
                mpf_mul(tx,tx,zmod);
                mpf_mul(ty,ty,zmod);
                mpf_add(x,x,tx);
                mpf_add(y,y,ty);
                mpf_div_ui(zoom, zoom, 4);
            } sprite.setScale(scale,scale);
            mpf_clear(modx); mpf_clear(mody);
            mpf_clear(zmod); mpf_clear(tx); mpf_clear(ty);
            mpf_clear(wRatio); mpf_clear(hRatio);
            mpf_clear(ratio);
            app.clear();
            app.draw(sprite);
            app.display();
            resetNums();
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        { bailPow++; mpf_set_ui(bail, (1 << bailPow)); resetNums(); while(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){} }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && bailPow > 1)
        { bailPow--; mpf_set_ui(bail, (1 << bailPow)); resetNums(); while(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){} }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        { reset(); resetNums(); while(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){} }

        draw = false;
        launchBots;
        waitBots;

        if(draw)
        { /// RESET
            scale = 1;
            sprite.setOrigin(0, 0);
            sprite.setPosition(0, 0);
            sprite.setScale(1,1);
            drawPixels;
        }
    }
    return EXIT_SUCCESS;
}
