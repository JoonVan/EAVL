#ifndef EAVL_RT_UTIL_H
#define EAVL_RT_UTIL_H
#include "eavlExecutor.h"
#include "eavlVector3.h"
#include <algorithm>    
using namespace std;
#define PI          3.14159265359f


EAVL_HOSTDEVICE float rcp(float f){ return 1.0f/f;}
EAVL_HOSTDEVICE float rcp_safe(float f) { return rcp((fabs(f) < 1e-8f) ? 1e-8f : f); }

enum primitive_t { TRIANGLE = 0, SPHERE = 1 , TET = 2 , CYLINDER = 3};

template <class T>  void deleteClassPtr(T * &ptr)
{
    if(ptr != NULL)
    {
        delete ptr;
        ptr = NULL;
    }
}

template <class T>  void deleteArrayPtr(T * &ptr)
{
    if(ptr != NULL)
    {
        delete[] ptr;
        ptr = NULL;
    }
}


struct RayGenFunctor
{
    int w;
    int h; 
    eavlVector3 nlook;// normalized look
    eavlVector3 delta_x;
    eavlVector3 delta_y;

    RayGenFunctor(int width, int height, float half_fovX, float half_fovY, eavlVector3 look, eavlVector3 up, float _zoom)
        : w(width), h(height)
    {
        float thx = tan(half_fovX*PI/180);
        float thy = tan(half_fovY*PI/180);


        eavlVector3 ru = up%look;
        ru.normalize();

        eavlVector3 rv = ru%look;
        rv.normalize();

        delta_x = ru*(2*thx/(float)w);
        delta_y = rv*(2*thy/(float)h);
        
        if(_zoom > 0)
        {
            delta_x /= _zoom;
            delta_y /= _zoom;    
        }
        

        nlook.x = look.x;
        nlook.y = look.y;
        nlook.z = look.z;
        nlook.normalize();

    }

    EAVL_FUNCTOR tuple<float,float, float> operator()(int idx){
        int i=idx%w;
        int j=idx/w;

        eavlVector3 ray_dir=nlook+delta_x*((2*i-w)/2.0f)+delta_y*((2*j-h)/2.0f);
        ray_dir.normalize();

        return tuple<float,float,float>(ray_dir.x,ray_dir.y,ray_dir.z);

    }

};

struct ScreenDepthFunctor{

    float proj22;
    float proj23;
    float proj32;

    ScreenDepthFunctor(float _proj22, float _proj23, float _proj32)
        : proj22(_proj22), proj23(_proj23), proj32(_proj32) 
    {
        
    }                                                 
    EAVL_FUNCTOR tuple<float> operator()(float depth){
       
        float projdepth = (proj22 + proj23 / (-depth)) / proj32;
        projdepth = .5 * projdepth + .5;
        return tuple<float>(projdepth);
    }
};

/*----------------------Utility Functors---------------------------------- */
struct ThreshFunctor
{
    int thresh;
    ThreshFunctor(int t)
        : thresh(t)
    {}

    EAVL_FUNCTOR tuple<int> operator()(int in){
        int out;
        if(in<thresh) out=0;
        else out=1;
        return tuple<int>(out);
    }

};

// 1 value to 3 arrays this uses a dummy array as input;
struct FloatMemsetFunctor1to3
{
    float value;
    FloatMemsetFunctor1to3(const float v)
        : value(v)
    {}

    EAVL_FUNCTOR tuple<float,float,float> operator()(float r){
        return tuple<float,float,float>(value,value,value);
    }

   

};

struct FloatMemsetFunctor
{
    float value;
    FloatMemsetFunctor(const float v)
        : value(v)
    {}

    EAVL_FUNCTOR tuple<float> operator()(float r){
        return tuple<float>(value);
    }

   

};

struct IntMemsetFunctor
{
    const int value;
    IntMemsetFunctor(const int v)
        : value(v)
    {}

    EAVL_FUNCTOR tuple<int> operator()(int dummy){
        return tuple<int>(value);
    }

   

};

//three values to three arrays
struct FloatMemsetFunctor3to3
{
    const float value1;
    const float value2;
    const float value3;
    FloatMemsetFunctor3to3(const float v1,const float v2, const float v3)
        : value1(v1),value2(v2),value3(v3)
    {}

    EAVL_FUNCTOR tuple<float,float,float> operator()(float r){
        return tuple<float,float,float>(value1,value2,value3);
    }

   

};

struct FloatMemcpyFunctor3to3
{
    FloatMemcpyFunctor3to3(){}

    EAVL_FUNCTOR tuple<float,float,float> operator()(tuple<float,float,float> input){
        return tuple<float,float,float>(get<0>(input),get<1>(input),get<2>(input));
    }

   

};

struct CopyFrameBuffer
{
    CopyFrameBuffer(){}

    EAVL_FUNCTOR tuple<byte,byte,byte,byte> operator()(tuple<float,float,float,float> input){
        return tuple<byte,byte,byte,byte>(get<0>(input)*255.,get<1>(input)*255.,get<2>(input)*255.,255);
    }

   

};

struct FloatMemcpyFunctor4to4
{
    FloatMemcpyFunctor4to4(){}

    EAVL_FUNCTOR tuple<float,float,float,float> operator()(tuple<float,float,float,float> input){
        return tuple<float,float,float,float>(get<0>(input),get<1>(input),get<2>(input),get<3>(input));
    }

   

};

struct FloatMemcpyFunctor1to1
{
    FloatMemcpyFunctor1to1(){}

    EAVL_FUNCTOR tuple<float> operator()(tuple<float> input){
        return tuple<float>(get<0>(input));
    }
};
struct IntMemcpyFunctor1to1
{
    IntMemcpyFunctor1to1(){}

    EAVL_FUNCTOR tuple<int> operator()(tuple<int> input){
        return tuple<int>(get<0>(input));
    }
};



struct AccFunctor3to3
{
    AccFunctor3to3(){}

    EAVL_FUNCTOR tuple<float,float,float> operator()(tuple<float,float,float,float,float,float> input){
        eavlVector3 color1(get<0>(input),get<1>(input),get<2>(input));
        eavlVector3 color2(get<3>(input),get<4>(input),get<5>(input));
        color1=color1+color2;
        return tuple<float,float,float>(min(color1.x,1.0f),min(color1.y,1.0f),min(color1.z,1.0f));
    }

};
struct AccFunctor1to1
{
    AccFunctor1to1(){}

    EAVL_FUNCTOR tuple<float> operator()(tuple<float,float> input){
        float color=get<0>(input)+get<1>(input);
        return tuple<float>(min(color,1.0f));
    }

};


struct WeightedAccFunctor1to1
{
    int samples;
    int additionalSamples;

    WeightedAccFunctor1to1(int _numSamples, int _additionalSamples)
    {
        samples=_numSamples;
        additionalSamples=_additionalSamples;
    }

    EAVL_FUNCTOR tuple<float> operator()(tuple<float,float> input){

        float newAve=get<0>(input)*samples+get<1>(input)*additionalSamples;
        newAve=newAve/(samples+additionalSamples);
        return tuple<float>(newAve);
    }

};

struct HitFilterFunctor
{
    HitFilterFunctor()
    {}

    EAVL_FUNCTOR tuple<int> operator()(int in){
        int out;
        if(in==-2) out=-1;
        else out=in;
        return tuple<int>(out);
    }

};

struct ConvertFrameBufferFunctor
{
    ConvertFrameBufferFunctor()
    {}

    EAVL_FUNCTOR tuple<byte,byte,byte,byte> operator()(tuple<float,float,float,float> in){
        float r = get<0>(in);
        float g = get<1>(in);
        float b = get<2>(in);
        float a = get<3>(in);
        byte  rb = r*255;
        byte  bb = b*255;
        byte  gb = g*255;
        byte  ab = a*255; 
        return tuple<byte,byte,byte,byte>(rb,gb,bb,ab);
    }

};

    
inline void writeBMP(int _height, int _width, eavlFloatArray *r, eavlFloatArray *g, eavlFloatArray *b,const char*fname)
{
    FILE *f;
    int size=_height*_width;
    unsigned char *img = NULL;
    int filesize = 54 + 3*_width*_height;  //w is your image width, h is image height, both int
    if( img )
        free( img );
    img = (unsigned char *)malloc(3*_width*_height);
    memset(img,0,sizeof(img));

    for(int j=size;j>-1;j--)
    {
        img[j*3  ]= (unsigned char) (int)(b->GetValue(j)*255);
        img[j*3+1]= (unsigned char) (int)(g->GetValue(j)*255);
        img[j*3+2]= (unsigned char) (int)(r->GetValue(j)*255);

    }

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       _width    );
    bmpinfoheader[ 5] = (unsigned char)(       _width>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       _width>>16);
    bmpinfoheader[ 7] = (unsigned char)(       _width>>24);
    bmpinfoheader[ 8] = (unsigned char)(       _height    );
    bmpinfoheader[ 9] = (unsigned char)(       _height>> 8);
    bmpinfoheader[10] = (unsigned char)(       _height>>16);
    bmpinfoheader[11] = (unsigned char)(       _height>>24);

    f = fopen(fname,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<_height; i++)
    {
        fwrite(img+(_width*(_height-i-1)*3),3,_width,f);
        fwrite(bmppad,1,(4-(_width*3)%4)%4,f);
    }
    fclose(f);
    free(img);
}


EAVL_HOSTDEVICE unsigned int expandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

/**
 * Count Leading Zeros
 * @param  value
 * @return int - number of leading zeros
 */
EAVL_HOSTDEVICE int clz(unsigned int value)
{
#ifdef __CUDA_ARCH__
    return __clz((int)value);
#else
    float fvalue = value;
    float result = 31 - floor(log2(fvalue));
    return (int) result;
#endif
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
EAVL_HOSTDEVICE unsigned int morton3D(float x, float y, float z)
{
    x = min(max(x * 1024.0f, 0.0f), 1023.0f);
    y = min(max(y * 1024.0f, 0.0f), 1023.0f);
    z = min(max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    unsigned int zz = expandBits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

inline unsigned int morton2D(float x, float y)
{
    x = min(max(x * 1024.0f, 0.0f), 1023.0f);
    y = min(max(y * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    return xx*4  + yy*2 ;
}

struct raySort{
    int id;
    unsigned int mortonCode;

}; 
struct tri
{
    float x[3];
    float y[3];
    float z[3];

    float xNorm[3];
    float yNorm[3];
    float zNorm[3];

    unsigned int mortonCode;

    float unitCentroid[3];

};

inline bool readBVHCache(float *&innerNodes, int &innerSize, float *&leafNodes, int &leafSize, const char* filename )
{
    ifstream bvhcache(filename, ios::in |ios::binary);
    if(bvhcache.is_open())
    {
        cout<<"Reading BVH Cache"<<endl;
        bvhcache.read((char*)&innerSize, sizeof(innerSize));
        if(innerSize<0) 
        {
            cerr<<"Invalid inner node array size "<<innerSize<<endl;
            bvhcache.close();
            return false;
        }
        innerNodes= new float[innerSize];
        bvhcache.read((char*)innerNodes, sizeof(float)*innerSize);

        bvhcache.read((char*)&leafSize, sizeof(leafSize));
        if(leafSize<0) 
        {
            cerr<<"Invalid leaf array size "<<leafSize<<endl;
            bvhcache.close();
            delete innerNodes;
            return false;
        }

        leafNodes= new float[leafSize];
        bvhcache.read((char*)leafNodes, sizeof(float)*leafSize);
    }
    else
    {
        cerr<<"Could not open file "<<filename<<" for reading bvh cache. Rebuilding..."<<endl;
        bvhcache.close();
        return false;
    }

    bvhcache.close();
    return true;
}

inline void writeBVHCache(const float *innerNodes, const int innerSize, const float * leafNodes, const int leafSize, const char* filename )
{
    cout<<"Writing BVH to cache"<<endl;
    ofstream bvhcache(filename, ios::out | ios::binary);
    
    if(bvhcache.is_open()) 
    {
        bvhcache.write((char*)&innerSize, sizeof(innerSize));
        bvhcache.write((const char*)innerNodes, sizeof(float)*innerSize);

        bvhcache.write((char*)&leafSize, sizeof(leafSize));
        bvhcache.write((const char*)leafNodes, sizeof(float)*leafSize);
    }
    else
    {
        cerr<<"Error. Could not open file "<<filename<<" for storing bvh cache."<<endl;
    }
    bvhcache.close();
    
}

inline bool spacialCompare(const raySort &lhs,const raySort &rhs)
{
    return lhs.mortonCode < rhs.mortonCode;
}

template<typename T>
EAVL_HOSTDEVICE bool solveQuadratic(const T &a, const T &b, const T &c, T &x0, T &x1)
{
    T discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        T q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) 
    {
        T tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    return true;
}


#endif