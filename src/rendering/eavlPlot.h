// Copyright 2010-2013 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_PLOT_H
#define EAVL_PLOT_H

#include "eavl.h"
#include "eavlView.h"
#include "eavlRenderer.h"
#include "eavlColorTable.h"

struct eavlPlot
{
    eavlDataSet  *data;
    string        colortable;
    int           cellset_index;
    int           variable_fieldindex;
    //int           variable_cellindex;
    eavlPseudocolorRenderer *pcRenderer;
    eavlSingleColorRenderer *meshRenderer;
    eavlCurveRenderer *curveRenderer;
    eavlBarRenderer *barRenderer;

    eavlPlot()
        : data(NULL),
          colortable("default"),
          cellset_index(-1),
          variable_fieldindex(-1),
          pcRenderer(NULL),
          meshRenderer(NULL),
          curveRenderer(NULL),
          barRenderer(NULL)
    {
    }
};

#endif