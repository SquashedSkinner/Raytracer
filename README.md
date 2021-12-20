# Raytracer
A program that utilises rays along with Ray-Sphere intersections to generate realistic renders of a virtual environment

To use the program, simply run either the release mode application or within the visual studio IDE.

It will provide you with 3 options; parallel, Non-parallel and References.

Choosing Non-Parallel will ask for the y pixel resolution, default all threadpool variables and use one thread.

Parallel will ask for the y pixel resolution, then let you select the number of threads (your maximum will be provided),
Number of Jobs for the thread pool to handle, and the Sample Rate.

Wait a bit and the program will ask you to enter the name of the rendered image as well as output the time taken to generate it.

The image can be found in [...Assignment/Raytracer/Raytracer]
