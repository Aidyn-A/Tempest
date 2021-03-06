/*!
 * \file flurry.cpp
 * \brief Main file to run a whole simulation
 *
 * Will make into a class in the future in order to interface with Python
 *
 * \author - Jacob Crabill
 *           Aerospace Computing Laboratory (ACL)
 *           Aero/Astro Department. Stanford University
 *
 * \version 0.0.1
 *
 * Flux Reconstruction in C++ (Flurry++) Code
 * Copyright (C) 2014 Jacob Crabill.
 *
 */

#include "tempest.hpp"

#ifndef _NO_MPI
#include <mpi.h>
#endif

int main(int argc, char *argv[]) {
  input params;
  geo Geo;
  solver Solver;

  int rank = 0;
  int nproc = 1;
#ifndef _NO_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
#endif
  params.rank = rank;
  params.nproc = nproc;

  if (rank == 0) {
    cout << endl;
    cout << " ======             Tempest            ======" << endl;
    cout << "          3D Overset Finite Volume Code      " << endl;
    cout << " Aerospace Computing Lab, Stanford University" << endl;
    cout << endl;
  }

  if (argc<2) FatalError("No input file specified.");

  setGlobalVariables();

  /* Read input file & set simulation parameters */
  params.readInputFile(argv[1]);

  /* Setup the mesh and connectivity for the simulation */
  Geo.setup(&params);

  /* Setup the solver, all elements and faces, and all FR operators for computation */
  Solver.setup(&params,&Geo);

  /* Stat timer for simulation (ignoring pre-processing) */
  simTimer runTime;
  runTime.startTimer();

  /* Write initial data file */
  writeData(&Solver,&params);

  //writeMeshTecplot(&Solver,&params);

#ifndef _NO_MPI
  // Allow all processes to finish initial file writing before starting computation
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  /* --- Calculation Loop --- */
  for (params.iter=params.initIter+1; params.iter<=params.iterMax; params.iter++) {

    Solver.update();

    if ((params.iter)%params.monitorResFreq == 0 || params.iter==1) writeResidual(&Solver,&params);
    if ((params.iter)%params.plotFreq == 0) writeData(&Solver,&params);

  }

  // Get simulation wall time
  runTime.stopTimer();
  runTime.showTime();

#ifndef _NO_MPI
 MPI_Finalize();
#endif
}
