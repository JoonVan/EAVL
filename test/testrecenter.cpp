// Copyright 2010-2014 UT-Battelle, LLC.  See LICENSE.txt for more information.
#include "eavl.h"
#include "eavlCUDA.h"
#include "eavlFilter.h"
#include "eavlDataSet.h"
#include "eavlTimer.h"
#include "eavlException.h"

#include "eavlImporterFactory.h"
#include "eavlVTKExporter.h"

#include "eavlCellToNodeRecenterMutator.h"
#include "eavlExecutor.h"


eavlDataSet *ReadWholeFile(const string &filename)
{
    eavlImporter *importer = eavlImporterFactory::GetImporterForFile(filename);
    
    if (!importer)
        THROW(eavlException,"Didn't determine proper file reader to use");

    string mesh = importer->GetMeshList()[0];
    eavlDataSet *out = importer->GetMesh(mesh, 0);
    vector<string> allvars = importer->GetFieldList(mesh);
    for (size_t i=0; i<allvars.size(); i++)
        out->AddField(importer->GetField(allvars[i], mesh, 0));

    return out;
}
 
void WriteToVTKFile(eavlDataSet *data, const string &filename,
        int cellSetIndex = 0)
{
    ofstream *p = new ofstream(filename.c_str());
    ostream *s = p;
    eavlVTKExporter exporter(data, cellSetIndex);
    exporter.Export(*s);
    p->close();
    delete p;
}

int main(int argc, char *argv[])
{
    try
    {   
        eavlExecutor::SetExecutionMode(eavlExecutor::PreferGPU);
        eavlInitializeGPU();

        if (argc != 3 && argc != 4)
            THROW(eavlException,"Incorrect number of arguments");

        // Read the input
        eavlDataSet *data = ReadWholeFile(argv[1]);

        eavlField *f = data->GetField(argv[2]);
        if (f->GetAssociation() != eavlField::ASSOC_CELL_SET)
            THROW(eavlException, "Wanted a cell-centered field.");

        string cellsetname = f->GetAssocCellSet();
        int cellsetindex = data->GetCellSetIndex(cellsetname);

        eavlCellToNodeRecenterMutator *cell2node = new eavlCellToNodeRecenterMutator;
        cell2node->SetDataSet(data);
        cell2node->SetField(argv[2]);
        cell2node->SetCellSet(data->GetCellSet(cellsetindex)->GetName());
        cell2node->Execute();

        if (argc == 4)
        {
            cerr << "\n\n-- done with surface normal, writing to file --\n";	
            WriteToVTKFile(data, argv[3], cellsetindex);
        }
        else
        {
            cerr << "No output filename given; not writing result\n";
        }


        cout << "\n\n-- summary of data set result --\n";	
        data->PrintSummary(cout);
    }
    catch (const eavlException &e)
    {
        cerr << e.GetErrorText() << endl;
        cerr << "\nUsage: "<<argv[0]<<" <infile.vtk> <fieldname> [<outfile.vtk>]\n";
        return 1;
    }


    return 0;
}
