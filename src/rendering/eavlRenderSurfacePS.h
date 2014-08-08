// Copyright 2010-2014 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_RENDER_SURFACE_PS_H
#define EAVL_RENDER_SURFACE_PS_H

#include "eavlRenderSurface.h"

class eavlRenderSurfacePS : public eavlRenderSurface
{
    ostringstream ps;
    string font;
    double width, height;
    set<int> imgdata_defines;
  public:
    eavlRenderSurfacePS()
    {
        //font = "Courier";
        font = "LiberationMono";
        width=0;
        height=0;
    }
    virtual void Initialize()
    {
        imgdata_defines.clear();
    }
    virtual void Resize(int w, int h)
    {
        ps.str("");
        ps.clear();
        Initialize();
        ps << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
        ps << "%%BoundingBox: 0 0 " << w << " " << h << endl;
        ps << "/DeviceRGB setcolorspace" << endl;
        width=w;
        height=h;
    }
    virtual void Activate()
    {
    }
    virtual void Finish()
    {
    }
    virtual void Clear(eavlColor bg)
    {
        ps << "newpath" << endl;
        ps << "0 0 moveto" << endl;
        ps << width<<" 0 lineto" << endl;
        ps << width<<" "<<height<<" lineto" << endl;
        ps << "0 "<<height<<" lineto" << endl;
        ps << "closepath" << endl;
        ps << bg.c[0] << " " << bg.c[1] << " " << bg.c[2] << " setrgbcolor" << endl;
        ps << "fill" << endl;
    }

    virtual void SetView(eavlView &v)
    {
    }
    virtual void SetViewportClipping(eavlView &v, bool clip)
    {
    }

    virtual void AddRectangle(float x, float y, 
                              float w, float h,
                              eavlColor c)
    {
        float x0 = (.5 + x*.5) * width;
        float y0 = (.5 + y*.5) * height;
        float x1 = (.5 + (x+w)*.5) * width;
        float y1 = (.5 + (y+h)*.5) * height;

        ps << "newpath" << endl;
        ps << x0 << " " << y0 << " moveto" << endl;
        ps << x1 << " " << y0 << " lineto" << endl;
        ps << x1 << " " << y1 << " lineto" << endl;
        ps << x0 << " " << y1 << " lineto" << endl;
        ps << "closepath" << endl;
        ps << c.c[0] << " " << c.c[1] << " " << c.c[2] << " setrgbcolor" << endl;
        ps << "fill" << endl;
    }
    virtual void AddLine(float x0, float y0,
                         float x1, float y1,
                         float linewidth,
                         eavlColor c)
    {
        x0 = (.5 + x0*.5) * width;
        y0 = (.5 + y0*.5) * height;
        x1 = (.5 + x1*.5) * width;
        y1 = (.5 + y1*.5) * height;

        ps << "newpath" << endl;
        ps << x0 << " " << y0 << " moveto" << endl;
        ps << x1 << " " << y1 << " lineto" << endl;
        ps << c.c[0] << " " << c.c[1] << " " << c.c[2] << " setrgbcolor" << endl;
        ps << linewidth << " setlinewidth" << endl;
        ps << "stroke" << endl;
    }
    virtual void AddColorBar(float x, float y, 
                             float w, float h,
                             string ctname,
                             bool horizontal)
    {
        eavlColorTable ct(ctname);
        int n = 1024;
        x = (.5 + x*.5) * width;
        y = (.5 + y*.5) * height;
        w = (w*.5) * width;
        h = (h*.5) * height;
        ps << "gsave" << endl;
        if (imgdata_defines.count(n) == 0)
        {
            ps << "/imgdata"<<n<<" "<<n<<" 3 mul string def" << endl;
            imgdata_defines.insert(n);
        }
        ps << x << " " << y << " translate" << endl;
        ps << w << " " << h << " scale" << endl;
        ps << n << " 1 8 ["<<n<<" 0 0 1 0 0]" << endl;
        ps << "{ currentfile imgdata"<<n<<" readhexstring pop }" << endl;
        ps << "false 3 colorimage" << endl;
        for (int i=0; i<n; ++i)
        {
            float val = float(i)/float(n-1);
            eavlColor c = ct.Map(val);
            unsigned char r,g,b,a;
            c.GetRGBA(r,g,b,a);
            char tmp[256];
            sprintf(tmp, "%2.2X%2.2X%2.2X", r,g,b);
            ps << tmp;
        }
        ps << endl;
        ps << "grestore" << endl;
    }

    virtual void AddText(float x, float y,
                         float scale,
                         float angle,
                         float windowaspect,
                         float anchorx, float anchory,
                         eavlColor color,
                         string text) ///<\todo: better way to get view here!
    {
        x = (.5 + x*.5) * width;
        y = (.5 + y*.5) * height;

        // select font and size
        ps << "/" << font << endl;
        int intscale = scale * height / 2.;
        ps << intscale << " selectfont" << endl;

        // set color
        ps << color.c[0] << " " << color.c[1] << " " << color.c[2] << " setrgbcolor" << endl;

        ps << "gsave" << endl;
        // move to anchor position
        ps << x << " " << y << " moveto" << endl;
        // shift up slightly to account for baseline
        ps << "0 " << float(intscale) * 0.15 << " rmoveto" << endl;
        ps << "(" << text << ") ";

        // rotation is around the anchor point, so it goes before the alignment
        ps << angle << " rotate ";

        // horizontal anchor
        ps << "dup stringwidth pop "<<(.5+.5*anchorx)<<" mul neg "
           << (.5+.5*anchory)*float(-intscale)<<" rmoveto ";
           //<< 0 <<" rmoveto ";
        ps << " show" << endl;
        ps << "grestore" << endl;
    }
    virtual void SaveAs(string fn, FileType ft)
    {
        if (ft != EPS)
        {
            THROW(eavlException, "Can only save PS images as EPS");
        }

        ofstream out(fn.c_str());
        //out << ps.rdbuf() << std::flush; // why doesn't this work?
        out << ps.str();
        out.close();
    }
};

#endif