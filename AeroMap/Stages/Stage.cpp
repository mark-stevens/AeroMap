// Stage.cpp
// Base class for all stages.
//

#include "Stage.h"

void Stage::BenchmarkStart()
{
	m_Timer.start();
}

void Stage::BenchmarkStop(const char* name, bool init)
{
	double sec = m_Timer.elapsed() * 0.001;

	XString file_name = XString::CombinePath(GetProject().GetDroneOutputPath(), "benchmark.txt");

	FILE* pFile = nullptr;
	if (init)
		pFile = fopen(file_name.c_str(), "wt");
	else
		pFile = fopen(file_name.c_str(), "at");

	if (pFile)
	{
		fprintf(pFile, "%s: %0.3f\n", name, sec);
		fclose(pFile);
	}
}


bool Stage::Rerun()
{
	// Force stage to run regardless of output
	// file state.
	//

	return true;
}