/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2021, OPEN AI LAB
 * Author: xwwang@openailab.com
 */

#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "common.h"
#include "tengine/c_api.h"
#include "tengine_operations.h"

typedef struct BoxInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;

std::vector<double> linspace(double start_in, double end_in, int num_in)
{
    std::vector<double> linspaced;

    double start = static_cast<double>(start_in);
    double end = static_cast<double>(end_in);
    double num = static_cast<double>(num_in);

    if (num == 0) { return linspaced; }
    if (num == 1) 
    {
        linspaced.push_back(start);
        return linspaced;
    }

    double delta = (end - start) / (num - 1);

    for(int i=0; i < num-1; ++i)
    {
      linspaced.push_back(start + delta * i);
    }
    linspaced.push_back(end); // I want to ensure that start and end
                            // are exactly the same as the input

    return linspaced;
}



void get_input_data_cv(const char* image_file, float* input_data, int img_h, int img_w, const float* mean, const float* scale)
{
    cv::Mat sample = cv::imread(image_file, 1);
    cv::Mat img;

    if (sample.channels() == 1)
        cv::cvtColor(sample, img, cv::COLOR_GRAY2RGB);
    else
        cv::cvtColor(sample, img, cv::COLOR_BGR2RGB);

    /* resize process */
    cv::resize(img, img, cv::Size(img_w, img_h));
    img.convertTo(img, CV_32FC3);
    float* img_data = (float*)img.data;

    /* nhwc to nchw */
    for (int h = 0; h < img_h; h++)
    {
        for (int w = 0; w < img_w; w++)
        {
            for (int c = 0; c < 3; c++)
            {
                int in_index = h * img_w * 3 + w * 3 + c;
                int out_index = c * img_h * img_w + h * img_w + w;
                input_data[out_index] = (img_data[in_index] - mean[c]) * scale[c];
            }
        }
    }
}

int main(int argc, char* argv[])
{
	std::vector<std::int16_t> culane_row_anchor = {121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228, 238, 248, 258, 267, 277, 287};
	int horizon_idx = 200;    
	const char* model_file = "model.tmfile";
    const char* image_file = "0992.jpg";
    int img_h = 288;
    int img_w = 512;
    int img_c = 3;
    const float mean[3] = {0.485f*255.f, 0.456f*255.f, 0.406f*255.f};
    const float scale[3] = {1/0.229f/255.f, 1/0.224f/255.f, 1/0.225f/255.f};

    int repeat_count = 3;
    int num_thread = 1;

	// Logic
    int cuLaneGriding_num = 200;
    std::vector<double> linSpaceVector = linspace(0, 512 - 1, cuLaneGriding_num);
    double linSpace = linSpaceVector[1] - linSpaceVector[0];
	printf("%2.2f\n",linSpace);
    cv::Mat img = cv::imread(image_file, 1);
    if (img.empty())
    {
        fprintf(stderr, "cv::imread %s failed\n", image_file);
        return -1;
    }

    /* set runtime options */
    struct options opt;
    opt.num_thread = num_thread;
    opt.cluster = TENGINE_CLUSTER_ALL;
    opt.precision = TENGINE_MODE_FP32;
    opt.affinity = 255;

    /* inital tengine */
    if (init_tengine() != 0)
    {
        fprintf(stderr, "Initial tengine failed.\n");
        return -1;
    }
    fprintf(stderr, "tengine-lite library version: %s\n", get_tengine_version());

	graph_t graph = create_graph(NULL, "tengine", model_file);
    if (graph == nullptr)
    {
        fprintf(stderr, "Create graph failed.\n");
        return -1;
    }

    int img_size = img_h * img_w * img_c;
    int dims[] = {1, 3, img_h, img_w};
    std::vector<float> input_data(img_size);

    tensor_t input_tensor = get_graph_input_tensor(graph, 0, 0);
    if (input_tensor == nullptr)
    {
        fprintf(stderr, "Get input tensor failed\n");
        return -1;
    }

    if (set_tensor_shape(input_tensor, dims, 4) < 0)
    {
        fprintf(stderr, "Set input tensor shape failed\n");
        return -1;
    }

    if (set_tensor_buffer(input_tensor, input_data.data(), img_size * sizeof(float)) < 0)
    {
        fprintf(stderr, "Set input tensor buffer failed\n");
        return -1;
    }

    /* prerun graph, set work options(num_thread, cluster, precision) */
    if (prerun_graph_multithread(graph, opt) < 0)
    {
        fprintf(stderr, "Prerun multithread graph failed.\n");
        return -1;
    }

    /* prepare process input data, set the data mem to input tensor */
    float input_scale = 0.f;
    int input_zero_point = 0;
    get_input_data_cv(image_file, input_data.data(), img_h, img_w, mean, scale);

    /* run graph */
    double min_time = DBL_MAX;
    double max_time = DBL_MIN;
    double total_time = 0.;
    for (int i = 0; i < repeat_count; i++)
    {
        double start = get_current_time();
        if (run_graph(graph, 1) < 0)
        {
            fprintf(stderr, "Run graph failed\n");
            return -1;
        }
        double end = get_current_time();
        double cur = end - start;
        total_time += cur;
        min_time = std::min(min_time, cur);
        max_time = std::max(max_time, cur);
    }
    fprintf(stderr, "Repeat %d times, thread %d, avg time %.2f ms, max_time %.2f ms, min_time %.2f ms\n", repeat_count, num_thread,
            total_time / repeat_count, max_time, min_time);
    fprintf(stderr, "--------------------------------------\n");

    tensor_t output_tensor = get_graph_output_tensor(graph, 0, 0);
	float* output_data = (float*)get_tensor_buffer(output_tensor);
	
	std::vector<BoxInfo> result;
	float out[201][18][4];
    float out1[18][4];
    float out2[18][4];

	for (int y = 0; y < 18; y++) {
        for (int l = 0; l < 4; l++) {
            for(int x =0;x < 201 ;x ++) {
                //cv::Mat c_data = data.channels(x);
                //const float xdata = c_data.row(y)[l] ;//c_data[ 4 * y + l];
				const float xdata = output_data[ x * 4 * 18 + 4 * y + l] ;//c_data[ 4 * y + l];
                //int idx = l+ 18*y + 201 * x;
                float expp = expf(xdata);
                out[x][y][l] = expp;
                if (x!=0) {
                    out1[y][l] += expp;
                } else {//==0
                    out1[y][l] = expp;
                }
            }
        }
    }

	for (int y =0;y < 18 ;y ++){
        //const float *row_data= data.row(y);
        for (int l =0;l < 4 ;l ++){
            float horizon_sum = 0;
            float horizon_max = 0;
            int horizon_max_idx = 0;
            for(int x =0;x < 201 ;x ++) {
                if (out1[y][l]!=0){
                    float o = out[x][y][l];
                    o /= out1[y][l] ;
                    if(o>horizon_max){
                        horizon_max = o;
                        horizon_max_idx = x;
                    }
                    //out2 = np.sum(prob * idx, axis=0)
                    o *=(float)x;
                    //out[x][y][l] = o;
                    /*
                     out[x][y][l] /= sum_exp ;
                    out[x][y][l] *=(float)x;
                     */
                    if (x!=0) {
                        out2[y][l] += o;
                    } else {//==0
                        out2[y][l] = o;
                    }
                    //horizon_sum +=o;
                }
            }
            if(horizon_max_idx==200){
                out2[y][l] = 0;
            }
        }
    };

    if (horizon_idx == 2011){
        //no result
    }else{
        for (int l =0;l < 4 ;l ++) {
            float sum = 0;
            for (int y = 0; y < 18; y++) {
                sum += out2[ y][ l];
            }
            if (sum > 2) {
                for (int y = 0; y < 18; y++) {
                    if (out2[y][l] > 0) {
                        BoxInfo box;
                        box.label = 1000+l;
                        box.score = out2[y][l];
						float xx = (out2[y][l] * 2.57 * 1280 / 512.f); //2.57 = linSpace = linSpaceVector[1] - linSpaceVector[0]; 
                        float yy = 720 * culane_row_anchor[y] / 288.f;
                        box.x1 = xx;
                        box.y1 = yy;
                        box.x2 = xx + 1;
                        box.y2 = yy + 1;
                        result.push_back(box);
                    }
                }
            }
        }
    }	

	for (auto &box:result) {
			cv::circle(img, cv::Point(box.x1, box.y1), 3, cv::Scalar(255, 0, 0), -1);
    }

	cv::imshow("LSTR", img);
	cv::resize(img, img, cv::Size(512, 288));
	cv::imwrite("out.jpg",img);
	// quit on x button
	cv::waitKey(0);

    /* release tengine */
    postrun_graph(graph);
    destroy_graph(graph);
    release_tengine();
}
