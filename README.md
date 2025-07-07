# Inference and Simulation of AML Disease Progression Models

Reza Mousavi, Moaath K. Mustafa Ali, and Daniel Lobo
<br>


## Building
Two different solutions are included:
* Evolution: evolutionary algorithm for the inference of models
* SearchViewer: user interface for visualizing the evolutionary algorithm results

Open each solution and compile them with Microsoft Visual Studio. Make sure the required dependencies are installed in your computer.

## Dependencies
* Microsoft Visual Studio
* Eigen
* Qt
* Qwt

## Implementation
The `Src` folder contains all the source code, implemented in C++. It is further structured into the following folders:

### `Common` 
This folder includes helper classes to handle files, save logs, and perform general mathematical methods.

### `DB` 
This folder contains the code for	accessing the database file employed to save the settings and results of the evolutionary algorithm. These include the parameters needed to run the method, the models created during the evolution, and the evolutionary statistics computed during the execution. The viewer can open these files to display the results of an evolutionary run.

### `Experiment` 
This folder includes the code to specify the clinical data that are defined in each of the patient histories (called experiment).

### `Model` 
This folder includes the implementation of the models. A model includes a set of nodes and their parameters and a set of links specifying the type of regulatory interactions between two nodes and their parameters.

### `Search` 
This folder contains the code for loading the parameters of the evolutionary algorithm, handling the model populations in the different islands, generating new models by executing evolutionary operators (crossover and mutation), and selecting the next generation populations. The folder also includes an implementation of the model fitness calculator that runs in the CPU as a multi-thread application.

### `Simulator` 
This folder includes the simulator that runs in the CPU. This includes the implementation for loading parameters related to the simulation, loading the models defined as classes into a system of ODEs for simulation, and performing the numerical computations to solve the system.

### `UI` 
The UI folder contains the user interface for both the evolution and viewer. The evolution program is run with a command line interface that uses a multi-thread implementation to maximize the performance. The viewer includes a graphical user interface to visualize the results of the evolution and perform simulations of the discovered models.
