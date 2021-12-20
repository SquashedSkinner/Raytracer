#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <limits>
#include "Sphere.h"
#include "Collisions.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::mutex mtx;

glm::vec3 diffuse_Ray()
{
	//Initialise a diffuse Ray and its random direction
	while (true)
	{
		glm::vec3 point(-1.0f + (2.0f * (rand() / (RAND_MAX + 1.0f))),
			-1.0f + (2.0f * (rand() / (RAND_MAX + 1.0f))),
			-1.0f + (2.0f * (rand() / (RAND_MAX + 1.0f))));
		if ((point[0] * point[0]) + (point[1] * point[1]) + (point[2] * point[2]) >= 1.0f)
		{
			continue;
		}
		return glm::normalize(point);	
	}
}



glm::vec3 colour(const Ray& r, Collisions environment, int bounces)
{
	contact record;
	
	if (bounces <= 0)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	// detect any collisions with the environment
	if (environment.hit(r, 0.001f, std::numeric_limits<float>::infinity(), record))
	{
		//act on object with diffuse material
		if (record.mat_rec == 0)
		{
			glm::vec3 target = record.point + record.normal + diffuse_Ray();
			return record.col_rec * colour(Ray(record.point, target - record.point), environment, bounces - 1);
		}
		// act on an object with metallic (reflective) material
		else if (record.mat_rec == 1)
		{
			glm::vec3 reflected = reflect(glm::normalize(r.direction), record.normal);
			return record.col_rec * colour(Ray(record.point, reflected + (record.fuzz_rec * diffuse_Ray())), environment, bounces - 1);
		}
	}
	glm::vec3 unit_d = glm::normalize(r.direction);
	float t = 0.5f * (unit_d.y + 1.0f);
	return ((1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f)) + (t * glm::vec3(0.5f, 0.7f, 1.0f));
}

float clamp(float x, float min, float max)
{

	// Returns max or min values if x surpasses the limits
	if (x < min)
	{
		return min;
	}

	if (x > max)
	{
		return max;
	}
	return x;
}

glm::vec3 reflect(const glm::vec3 vec, const glm::vec3 n)
{
	//Reflects the Ray off a surface
	return vec - 2 * dot(vec, n) * n;
}

void drawImage(int numJobs, Collisions environment, int x_pix, int y_pix, uint8_t* pixels, int jobID, std::vector<int>* job_list, int samples, int bounces)
{
	// Parallel programming
	int thread = jobID + 1;
	bool finished = false;

	//lock the mutex
	mtx.lock();
	//set new job and compute
	job_list->at(jobID) = job_list->back();
	//confirm job completion
	job_list->pop_back();
	//unlock mutex for next process
	mtx.unlock();
	

	do
	{
		//Screen resolution
		float imageHeight = 2.0f;
		// Aspect ratio
		float imageWidth = (16.0f / 9.0f) * imageHeight;
		glm::vec3 zero(0.0f, 0.0f, 0.0f);
		// Horizontal
		glm::vec3 h (imageWidth, 0.0f, 0.0f);
		// Vetical
		glm::vec3 v (0.0f, imageHeight, 0.0f);
		// Back left
		glm::vec3 bL = zero - h / 2.0f - v / 2.0f - glm::vec3(0.0f, 0.0f, 1.0f);

		// Declare job size and set them to the amount of pixel height of the image divided by the number
		// of jobs previously declared
		int jobs_H = static_cast<int>(y_pix / numJobs);
		int start = (jobs_H * (numJobs - jobID)) - 1;
		int finish = jobs_H * (numJobs - jobID - 1);
		//Find remaining jobs
		int remainder = y_pix % numJobs;
		int index = ((jobs_H + remainder) * x_pix * 3) + (jobs_H * x_pix * 3 * (jobID - 1));

		if (jobID == 0)
		{
			start += remainder;
			index = 0;
		}
		for (int j = start; j >= finish; j--)
		{
			for (int i = 0; i < x_pix; i++)
			{
				glm::vec3 col = glm::vec3(0.3f, 0.0f, 0.0f);
				for (int s = 0; s < samples; s++)
				{
					// Random ray
					float a = (float(i) + rand() / (RAND_MAX + 1.0f)) / x_pix;
					float b = (float(j) + rand() / (RAND_MAX + 1.0f)) / y_pix;
					Ray r(zero, bL + a * h + v * b);
					col += colour(r, environment, bounces);
				}

				int x = 0;
				do
				{
					// Assign colours
					col[x] = sqrt(col[x] / samples);
					pixels[index++] = int(255.99f * clamp(col[x], 0.0f, 0.99f));
					x++;
				} while (x != 3);
			}
		}
		finished = true;

		mtx.lock();
		if(job_list->size() > 0)
		{
			jobID = job_list->back();
			job_list->pop_back();
			finished = false;
		}
		mtx.unlock();
	} while (finished == false);
}

double start(int numThreads, int samples, int bounces, int y_pix, int job_number)
{
	Collisions environment;

	// Background Sphere												 position			    radius			   colour		material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(0.0f, -100.5f, -1.0f), 100.0f, glm::vec3(0.8, 0.8, 0.0), 0, 0.0f));

	// Centre Sphere													 position			 radius			  colour		material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(0.0f, 0.5f, -1.0f), 0.4f, glm::vec3(1.0, 0.4, 0.3), 1, 0.0f));

	// Tiny Sphere 1														  position			 radius			  colour		material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(0.0f, -0.2f, -1.0f), 0.3f, glm::vec3(0.4, 0.9, 0.3), 0, 0.0f));

	// Left Sphere														  position			 radius			   colour		material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(-1.0f, 0.0f, -1.0f), 0.5f, glm::vec3(0.0, 1.0, 0.2), 0, 0.0f));

	// Left Sphere 2														 position		  radius		colour	         material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(-1.0f, 0.2f, -1.0f), 0.3f, glm::vec3(0.1, 0.1, 0.1), 1, 0.0f));


	// Right Sphere														  position			radius			  colour		material/fuzz
	environment.sceneObjects.push_back(std::make_shared<Sphere>(glm::vec3(1.0f, 0.0f, -1.0f), 0.5f, glm::vec3(0.1, 0.1, 0.1), 1, 0.0f));

	// Aspect ratio to find x_pix
	int x_pix = static_cast<int>(y_pix * (16.0f / 9.0f));
	uint8_t* pixels = new uint8_t[x_pix * y_pix * 3];

	// Define the static number of threads within a pool
	std::thread* threadArray = new std::thread[numThreads];

	// Search for a job for a thread
	std::vector<int>* job_list = new std::vector<int>;
	for (int i = 0; i < job_number; i++)
	{
		job_list->push_back(i);
	}

	//begin keeping a track of time
	std::chrono::steady_clock::time_point tTaken_A = std::chrono::high_resolution_clock::now();

	//send thread to commence job
	try
	{
		for (int i = 0; i < numThreads; i++)
		{
			threadArray[i] = std::thread(drawImage, job_number, environment, x_pix, y_pix, pixels, i, job_list, samples, bounces);
		}

		//assign new job once there is a free thread
		for (int i = 0; i < numThreads; i++)
		{
			threadArray[i].join();
		}
	}
	catch(...)
	{
		throw;
	}

	// Finding time taken to process image
	std::chrono::steady_clock::time_point tTaken_B = std::chrono::high_resolution_clock::now();
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tTaken_B - tTaken_A);
	int seconds = ms.count() / 1000;
	std::cout << "Elapsed time: " << seconds << " seconds\n";

	// Entering the image file parameters
	std::cout << "Please enter the name the file\n";
	std::string outputName;
	std::cin >> outputName;
	outputName = outputName + ".png";
	// Use stbi_write_png to save the image in the project folder
	stbi_write_png(outputName.c_str(), x_pix, y_pix, 3, pixels, x_pix * 3);
	std::cout << "Saved as: " << outputName << "\n";

	return ms.count();
}

int main()
{
	//deafult y value for small scale processes (PC drawbacks)
	int y_pix = 200;

	// Default values, will change during runtime if option 1 is selected.
	int job_number = 100;
	int sampleNo = 100;
	int bounces = 50;
	int numThreads = std::thread::hardware_concurrency(); //My PC thread max = 4, laptop = 2
	int average = 1;
	int loop = 0;

	bool correct = false;
	while (correct == false)
	{
		std::cout << "Please choose an option:\n1.) Parallelised Ray Tracing\n2.) Non-parallelised Ray Tracing\n\n3.) References\n";
		int choice;
		std::cin >> choice;
		if (choice == 1)
		{
			correct = 1;
			std::cout << "Please enter the pixel height of your image\n";
			std::cin >> y_pix;
			if (y_pix >= 800)
			{
				std::cout << "Warning: Image may take extended period to render, please enter the pixel height value to either confirm or change the value\n";
				std::cin >> y_pix;
			}
			std::cout << "Please enter the number of threads you wish to use (maximum threads for Hardware = " << numThreads << ")\n";
			std::cin >> numThreads;
			if (numThreads > std::thread::hardware_concurrency())
			{
				numThreads = std::thread::hardware_concurrency();
				std::cout << "Error, value not possible with system. Threads set to" << numThreads;				
			}	
			std::cout << "Please enter the number of jobs you wish to use (100 recommended)\n";
			std::cin >> job_number;
			if (job_number >= 250)
			{
				job_number = 100;
				std::cout << "Error, too many jobs selected. Defaulted value to" << job_number;
				
			}
			std::cout << "Please enter the number of samples you wish to use (>= 100 recommended)\n";
			std::cin >> sampleNo;
			if (sampleNo >= 500)
			{
				job_number = 500;
				std::cout << "Error, too many samples will increase process time exponentially. Jobs set to" << job_number << "\n";
			}

		}
		else if (choice == 2)
		{
			correct = 1;
			std::cout << "Please enter the pixel height of your image\n";
			std::cin >> y_pix;
			if (y_pix >= 800)
			{
				std::cout << "Warning: Image may take extended period to render, please enter the pixel height value to either confirm or change the value\n";
				std::cin >> y_pix;
			}
			numThreads = 1;
		}
		else if (choice == 3)
		{
			std::cout << "Shirley, P., 2020. Ray Tracing in One Weekend. [online] Raytracing.github.io. Available at: <https://Raytracing.github.io/books/RayTracingInOneWeekend.html#outputanimage> [Accessed 1 December 2021].\nNVIDIA Developer. 2021. Introducing the NVIDIA RTX Ray Tracing Platform. [online] Available at: <https://developer.nvidia.com/rtx/Raytracing> [Accessed 9 December 2021].\n\nScratchapixel.com. 2021. Introduction to Ray Tracing: a Simple Method for Creating 3D Images (Writing a Basic Ray Tracer). [online] Available at: <https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-Ray-tracing/Ray-tracing-practical-example> [Accessed 9 December 2021].\n\nNVIDIA Developer. 2021. Introducing the NVIDIA RTX Ray Tracing Platform. [online] Available at: <https://developer.nvidia.com/rtx/Raytracing> [Accessed 9 December 2021].\n\n";
		}
		else
		{
			correct = 0;
		}
		
	}
	std::cout << "Beginning image generation. Stand by...\n";
	if (loop == 1)
	{

	}
	else
	{
		start(numThreads, sampleNo, bounces, y_pix, job_number);
	}
	return 0;

}