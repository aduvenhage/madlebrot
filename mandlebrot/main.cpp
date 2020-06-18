#include "jpeg.h"
#include <iostream>
#include <complex>
#include <chrono>
#include <vector>
#include <cmath>


// Naive implementation
struct MandleBrot1
{
    static int value(int _iX, int _iY, int _iWidth, int _iHeight, int iMaxIterations)
    {
        std::complex<double> point((double)_iX/_iWidth*2.3-1.2, (double)_iY/_iHeight*2.3-1.0);
        std::complex<double> z(0, 0);
        unsigned int nb_iter = 0;
        
        while (abs(z) < 2 && nb_iter <= iMaxIterations)
        {
            z = z * z + point;
            nb_iter++;
        }
        
        if (nb_iter < iMaxIterations) return (nb_iter << 8) / (iMaxIterations-1);
        else return 0;
    }
    
    static void render(const char *_pszFilename, int _iWidth, int _iHeight)
    {
        unsigned char *image_buffer = (unsigned char*)malloc(_iWidth * _iHeight * 3);
        
        for (int y = 0; y < _iHeight; y++)
        {
            for (int x = 0; x < _iWidth; x++)
            {
                int color = value(x, y, _iWidth, _iHeight, 100);
                int ipx = (y * _iWidth + x) * 3;
                
                image_buffer[ipx] = color;
                image_buffer[ipx+1] = color << 1;
                image_buffer[ipx+2] = color << 2;
            }
        }
        
        writeJpegFile(_pszFilename, _iWidth, _iHeight, image_buffer, 100);
    }
    
    static void profile(const char *_pszFilename, int _iWidth, int _iHeight)
    {
        std::chrono::high_resolution_clock clk;
        auto tp1 = clk.now();
        render(_pszFilename, _iWidth, _iHeight);
        auto td = clk.now() - tp1;
        
        printf("Mandlebrot1: %.2f\n", std::chrono::duration_cast<std::chrono::milliseconds>(td).count()/1000.0);
    }
};


// optimised implementation
class MandleBrot2
{
 public:
    MandleBrot2(int _iWidth, int _iHeight)
        :m_iWidth(_iWidth),
         m_iHeight(_iHeight),
         m_iMaxIterations(0),
         m_image(m_iWidth * m_iHeight * 3, 0),
         m_dPosX(0),
         m_dPosY(0),
         m_dZoom(0),
         m_dScaleX(0),
         m_dScaleY(0)
    {
        setView(2.0, 1.5, 0.2);
    }
    
    void setView(double _dX, double _dY, double _dZoom)
    {
        m_dPosX = _dX;
        m_dPosY = _dY;
        m_dZoom = _dZoom;
        m_dScaleX = 1.0 / m_iWidth / m_dZoom;
        m_dScaleY = 1.0 / m_iHeight / m_dZoom;
        m_iMaxIterations = 200;
    }
        
    void render(const char *_pszFilename)
    {
        int ipx = 0;
        unsigned char *pImage = m_image.data();
        
        for (int y = 0; y < m_iHeight; y++)
        {
            for (int x = 0; x < m_iWidth; x++)
            {
                int color = value(x, y);
                
                pImage[ipx++] = color;
                pImage[ipx++] = color << 1;
                pImage[ipx++] = color << 2;
            }
        }
        
        writeJpegFile(_pszFilename, m_iWidth, m_iHeight, m_image.data(), 100);
    }
    
    void profile(const char *_pszFilename)
    {
        std::chrono::high_resolution_clock clk;
        auto tp1 = clk.now();
        render(_pszFilename);
        auto td = clk.now() - tp1;
        
        printf("Mandlebrot2: %.2f\n", std::chrono::duration_cast<std::chrono::milliseconds>(td).count()/1000.0);
    }
    
 private:
     int value(int _iX, int _iY)
     {
         double fCx = (double)_iX * m_dScaleX + m_dPosX;
         double fCy = (double)_iY * m_dScaleY + m_dPosY;
         double fZx = 0;
         double fZy = 0;
         unsigned int nb_iter = 0;
         
         while (nb_iter < m_iMaxIterations)
         {
             // the 'z = z*z + c' and '|z|' calculations are combined to speed things up
             double fZxx = fZx * fZx;
             double fZyy = fZy * fZy;
             
             fZy = 2 * fZx * fZy + fCy;
             fZx = fZxx - fZyy + fCx;
             
             if ((fZxx + fZyy) >= 4) {
                 return nb_iter << 4;
             }
             
             nb_iter++;
         }
         
         return 0;
     }
     
 private:
    int                         m_iWidth;
    int                         m_iHeight;
    int                         m_iMaxIterations;
    std::vector<unsigned char>  m_image;
    double                      m_dPosX;
    double                      m_dPosY;
    double                      m_dZoom;
    double                      m_dScaleX;
    double                      m_dScaleY;
};


int main(int argc, const char * argv[])
{
    int width = 4000;
    int height = 4000;

    // profile implementation speed
    MandleBrot1::profile("test1.jpeg", width, height);
    
    auto mb2 = MandleBrot2(width, height);
    mb2.setView(-1.5, -1.0, 0.4);
    mb2.profile("test2.jpeg");
    
    // play with zoom
    
    return 0;
}
