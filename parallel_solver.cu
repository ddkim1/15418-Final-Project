#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <cstring>
#include "minesweeper.h" 

#include <cuda.h> 
#include <cuda_runtime.h> 
#include <driver_functions.h> 

#include "CycleTimer.h" 

#define PART_SIDE 3 

__device__ bool 
cu_is_valid(int y, int x, int height, int width) { 
    return (y >= 0 && y < height && x >= 0 && x < width); 
} 

__device__ int 
cu_neighboring_mines(int y, int x, int* gpuSolverBoard, int height, int width) { 
    int neighboringmines = 0; 
    for (int dy = -1; dy < 2; dy++) { 
        for (int dx = -1; dx < 2; dx++) { 
            if (cu_is_valid(y + dy, x + dx, height, width) && gpuSolverBoard[(y + dy) * width + (x + dx)] == -1) { 
                // printf("blockIdx.x: %d\n", blockIdx.x); 
                neighboringmines++; 
            }
        }
    } 
    return neighboringmines; 
} 

__device__ int 
cu_unknown_tiles(int y, int x, int* gpuSolverBoard, int height, int width) { 
    int unknownTiles = 0; 
    for (int dy = -1; dy < 2; dy++) { 
        for (int dx = -1; dx < 2; dx++) { 
            if (cu_is_valid(y + dy, x + dx, height, width) && gpuSolverBoard[(y + dy) * width + (x + dx)] == -10) { 
                unknownTiles++; 
            } 
        } 
    } 
    return unknownTiles; 
} 

__device__ void 
cu_double_tap(int y, int x, int* gpuSolverBoard, int* tempSolverBoard, int height, int width, int* gpuBoard) { 
    for (int dy = -1; dy < 2; dy++) { 
        for (int dx = -1; dx < 2; dx++) { 
            if (cu_is_valid(y + dy, x + dx, height, width)) { 
                // printf("revealing blockIdx %d global_square_idx_y %d global_square_idx_x %d\n", blockIdx.x, y + dy, x + dx); 
                tempSolverBoard[(height * width * blockIdx.x) + (y + dy) * width + (x + dx)] = gpuBoard[(y + dy) * width + (x + dx)]; 
            } 
        }
    }
} 

__device__ void 
cu_deduce_mines(int y, int x, int* tempSolverBoard, int height, int width, int* gpuSolverMineLocations, int* cudaCountMineFound) { 
    for (int dy = -1; dy < 2; dy++) { 
        for (int dx = -1; dx < 2; dx++) { 
            if (cu_is_valid(y + dy, x + dx, height, width) && tempSolverBoard[(height * width * blockIdx.x) + (y + dy) * width + (x + dx)] == -10) { 
                // printf("revealing bomb at blockIdx %d global_square_idx_y %d global_square_idx_x %d\n", blockIdx.x, y + dy, x + dx); 
                tempSolverBoard[(height * width * blockIdx.x) + (y + dy) * width + (x + dx)] = -1; 
                int updated_idx = atomicAdd(cudaCountMineFound, 1); 
                gpuSolverMineLocations[updated_idx * 2] = y + dy; 
                gpuSolverMineLocations[updated_idx * 2 + 1] = x + dx; 
            } 
        } 
    } 
}

__global__ void 
moveDoubleTap_kernel(int* gpuBoard, 
                     int* gpuSolverBoard, 
                     int* gpuMineLocations, 
                     int* gpuSolverMineLocations, 
                     int height, 
                     int width, 
                     int part_length_x, 
                     int part_length_y, 
                     int* cudaCountMineFound, 
                     int* tempSolverBoard) { 
    // this actually perform both randomMove and the double tap function 
    // here, we first find the next randomly guessing entry 
    __shared__ int this_round_guess_pos[2]; 

    // int local_num_failure = 0; 
    int local_part_h_idx = blockIdx.x/part_length_x; 
    int local_part_v_idx = blockIdx.x%part_length_x; 
    int local_square_offset_y = local_part_h_idx * PART_SIDE; 
    int local_square_offset_x = local_part_v_idx * PART_SIDE; 
    bool backtrace = false; 
    /*
    int global_idx = blockIdx.x * blockDim.x + threadIdx.x; 
    if (global_idx == 0) { 
        printf("got in\n"); 
        for (int i = 0; i < 10; i++) { 
            printf("%d\n", gpuSolverBoard[i]); 
        }
    }
    __syncthreads(); 
    */ 

    // printf("blockIdx.x %d, threadIdx.x %d, local_part_h_idx %d, local_part_v_idx %d, local_square_offset_y %d, local_square_offset_x %d\n", blockIdx.x, threadIdx.x, local_part_h_idx, local_part_v_idx, local_square_offset_y, local_square_offset_x); 
    // printf("lots of print\n"); 
    if (threadIdx.x == 0) { 
        int randY = 0; 
        int randX = 0; 
        // one thread of each process would be used to determine which square in the corresponding part will be appointed. 
        for (int i = 0; i < PART_SIDE; i++)  { // y 
            for (int j = 0; j < PART_SIDE; j++) { // x 
                int global_square_idx_y = i + local_square_offset_y; 
                int global_square_idx_x = j + local_square_offset_x; 
                // if (blockIdx.x == 0 || blockIdx.x == 1) 
                    // printf("we can discuss implementation of block idx %d global_sq_y %d, global_sq_x\n", blockIdx.x, global_square_idx_x, global_square_idx_x); 
                if (gpuSolverBoard[global_square_idx_y * width + global_square_idx_x] == -10) { 
                    randY = global_square_idx_y;  // we use the local grid idx 
                    randX = global_square_idx_x;  // local grid idx 
                    // printf("blockIdx.x %d threadIdx.x %d, randY is %d, randX is %d\n", blockIdx.x, threadIdx.x, randY, randX); 
                    // break; 
                }
            }
        } 
        this_round_guess_pos[0] = randY; 
        this_round_guess_pos[1] = randX; 
        // then, we run through the performMove functionality 
        gpuSolverBoard[randY * width + randX] = gpuBoard[randY * width + randX]; 
        tempSolverBoard[(height * width * blockIdx.x) + randY * width + randX] = gpuBoard[randY * width + randX]; 

        if (gpuBoard[randY * width + randX] == -1) { 
            int updated_idx = atomicAdd(cudaCountMineFound, 1); 
            gpuSolverMineLocations[updated_idx * 2] = randY; 
            gpuSolverMineLocations[updated_idx * 2 + 1] = randX; 
            // printf("blockIdx.x %d random click is a bomb\n", blockIdx.x); 
        } 
    }
    __syncthreads(); 
    // printf("blockIdx.x %d, we have the chosen idx to be %d %d\n", blockIdx.x, this_round_guess_pos[0], this_round_guess_pos[1]); 

    // identify which square does each thread take charge of 
    int local_idx_y = threadIdx.x / PART_SIDE; 
    int local_idx_x = threadIdx.x % PART_SIDE; 
    int global_square_idx_y = local_idx_y + local_square_offset_y; 
    int global_square_idx_x = local_idx_x + local_square_offset_x; 
    int randY = this_round_guess_pos[0]; 
    int randX = this_round_guess_pos[1]; 
    // printf("blockIdx.x %d, threadIdx.x %d, global_square_idx_y %d, global_square_idx_x %d, randY %d, randX %d\n", blockIdx.x, threadIdx.x, global_square_idx_y, global_square_idx_x, randY, randX); 

    // here, we are left with block where they found that a new starting point has emerged 
    int neighborMines = cu_neighboring_mines(randY, randX, gpuSolverBoard, height, width); 
    int unknownSquares = cu_unknown_tiles(randY, randX, gpuSolverBoard, height, width); 
    // printf("blockIdx.x %d, threadIdx.x %d, neighborMines %d, unknownSquares %d\n", blockIdx.x, threadIdx.x, neighborMines, unknownSquares); 
    // if (blockIdx.x == 0 && threadIdx.x == 0) { 
        // printf("critical location bomb: %d\n", gpuSolverBoard[randY * width + randX]); 
    // }

    // double tap 
    if (neighborMines == gpuSolverBoard[randY * width + randX] || gpuSolverBoard[randY * width + randX] == 0) { 
        // double tap but using multi-thread 
        // printf("double tap situation is found for blockidx.x %d and randY %d randX %d\n", blockIdx.x, randY, randX); 
        // if (global_square_idx_y >= randY - 1 && global_square_idx_y < randY + 2 && global_square_idx_x >= randX - 1 && global_square_idx_x < randX + 2) { 
        //     printf("revealing blockIdx %d global_square_idx_y %d global_square_idx_x %d\n", blockIdx.x, global_square_idx_y, global_square_idx_x); 
        //     tempSolverBoard[(height * width * blockIdx.x) + (global_square_idx_y * width + global_square_idx_x)] = gpuBoard[global_square_idx_y * width + global_square_idx_x]; 
        // } 
        if (threadIdx.x == 0) { 
            cu_double_tap(randY, randX, gpuSolverBoard, tempSolverBoard, height, width, gpuBoard); 
        } 
        backtrace = true; 
    } 
    
    __syncthreads(); 

    // deduce mines 
    if (unknownSquares == gpuSolverBoard[randY * width + randX] - neighborMines) { 
        // printf("deduce mines situation is found for blockidx.x %d and randY %d randX %d\n", blockIdx.x, randY, randX); 
        // if (global_square_idx_y >= randY - 1 && global_square_idx_y < randY + 2 && global_square_idx_x >= randX - 1 && global_square_idx_x < randX + 2) { 
        //     if (gpuSolverBoard[global_square_idx_y * width + global_square_idx_x] == -10) { 
        //         printf("revealing bomb at blockIdx %d global_square_idx_y %d global_square_idx_x %d\n", blockIdx.x, global_square_idx_y, global_square_idx_x); 
        //         tempSolverBoard[(height * width * blockIdx.x) + (global_square_idx_y * width + global_square_idx_x)] = -1; 
        //         int updated_idx = atomicAdd(cudaCountMineFound, 1); 
        //         gpuSolverMineLocations[updated_idx * 2] = global_square_idx_y; 
        //         gpuSolverMineLocations[updated_idx * 2 + 1] = global_square_idx_x; 
        //         printf("all of them are bombs\n"); 
        //     } 
        // } 
        if (threadIdx.x == 0) { 
            cu_deduce_mines(randY, randX, tempSolverBoard, height, width, gpuSolverMineLocations, cudaCountMineFound); 
        } 
        backtrace = true; 
    } 
    __syncthreads(); 

    // printf("got after the heat check\n"); 

    return; 
} 
    
    
    // if (tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] != -10) { 
    //     // not occupied in the current block, but may be modified in other blocks 
    //     for (int i = 0; i < gridDim.x; i++) { 
    //         if (i == blockIdx.x) { 
    //             continue; 
    //         }
    //         if (tempSolverBoard[(height * width * i) + global_square_idx_y * width + global_square_idx_x] == -10) { 
    //             tempSolverBoard[(height * width * i) + global_square_idx_y * width + global_square_idx_x] = tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x]; 
    //         } 
    //     } 
    // } 

    // if (blockIdx.x == 3 && threadIdx.x == 2) { 
    //     for (int i = 0; i < gridDim.x; i++) { 
    //         printf("global_square_idx_y %d, i %d, global_square_idx_x %d, %d\n", global_square_idx_y, i, global_square_idx_x, tempSolverBoard[(height * width * i) + global_square_idx_y * global_square_idx_x]); 
    //     }
    // } 

    // return; 

__global__ void 
backtracing_kernel(int* gpuBoard, 
                     int* gpuSolverBoard, 
                     int* gpuMineLocations, 
                     int* gpuSolverMineLocations, 
                     int height, 
                     int width, 
                     int part_length_x, 
                     int part_length_y, 
                     int* cudaCountMineFound, 
                     int* tempSolverBoard, 
                     int* outcome) { 
    
    // bool backtrace = true; 
    int backtrace = 0; 
    int neighborMines; 
    int unknownSquares; 

    int local_part_h_idx = blockIdx.x/part_length_x; 
    int local_part_v_idx = blockIdx.x%part_length_x; 
    int local_square_offset_y = local_part_h_idx * PART_SIDE; 
    int local_square_offset_x = local_part_v_idx * PART_SIDE; 
    int local_idx_y = threadIdx.x / PART_SIDE; 
    int local_idx_x = threadIdx.x % PART_SIDE; 
    int global_square_idx_y = local_idx_y + local_square_offset_y; 
    int global_square_idx_x = local_idx_x + local_square_offset_x; 
    // int randY = this_round_guess_pos[0]; 
    // int randX = this_round_guess_pos[1]; 

    // big while loop is here 
    // backtrace = false; 
    if (tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] == -10) { 
        return; 
    } 
    neighborMines = cu_neighboring_mines(global_square_idx_y, global_square_idx_x, gpuSolverBoard, height, width); 
    unknownSquares = cu_unknown_tiles(global_square_idx_y, global_square_idx_x, gpuSolverBoard, height, width); 
    if (unknownSquares == 0) { 
        return; 
    } 
    if (neighborMines == tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] || tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] == 0) { 
        cu_double_tap(global_square_idx_y, global_square_idx_x, gpuSolverBoard, tempSolverBoard, height, width, gpuBoard); 
        backtrace = 1; 
    } 
    if (unknownSquares == tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] - neighborMines) { 
        cu_deduce_mines(global_square_idx_y, global_square_idx_x, tempSolverBoard, height, width, gpuSolverMineLocations, cudaCountMineFound); 
        backtrace = 1; 
    } 
        // if (tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] == -10) { 
        //     // not occupied in the current block, but may be modified in other blocks 
        //     for (int i = 0; i < blockDim.x; i++) { 
        //         if (i == blockIdx.x) { 
        //             continue; 
        //         }
        //         if (tempSolverBoard[(height * width * i) + global_square_idx_y * width + global_square_idx_x] != -10) { 
        //             tempSolverBoard[(height * width * blockIdx.x) + global_square_idx_y * width + global_square_idx_x] = tempSolverBoard[(height * width * i) + global_square_idx_y * width + global_square_idx_x]; 
        //         } 
        //     } 
        // } 
    *outcome = backtrace; 
} 

__global__ void 
update_gpu_solver_board(int* gpuSolverBoard, int* tempSolverBoard, int height, int width, int num_of_block) { 
    int idx = blockIdx.x * blockDim.x + threadIdx.x; 
    if (idx >= height * width) { 
        return; 
    } 
    int tempSolverEventualValue = -10; 
    for (int i = 0; i < num_of_block; i++) { 
        int idx_temp = idx + (i * height * width); 
        int tempSolverEntry = tempSolverBoard[idx_temp]; 
        if (gpuSolverBoard[idx] != tempSolverEntry && tempSolverEntry != -10) { 
            gpuSolverBoard[idx] = tempSolverEntry; 
            tempSolverEventualValue = tempSolverEntry; 
        } 
    } 
    if (tempSolverEventualValue != -10) { 
        for (int i = 0; i < num_of_block; i++) { 
            int idx_temp = idx + (i * height * width); 
            tempSolverBoard[idx_temp] = tempSolverEventualValue; 
        }
    } 
    return; 
}

void Minesweeper::parallel_cuda_solver() { 
    if (height % PART_SIDE != 0 || width % PART_SIDE != 0) { 
        // printf("got inside the exit statement if\n"); 
        exit(0); 
    } 
    int num_of_blocks = (height/PART_SIDE) * (width/PART_SIDE); 
    const int threadsPerBlock = PART_SIDE * PART_SIDE; 
    int part_length_y = height/PART_SIDE; 
    int part_length_x = width/PART_SIDE; 
    // printf("number_of_blocks %d, threadsPerBlock %d, part_length_y %d, part_length_x %d\n", num_of_blocks, threadsPerBlock, part_length_x, part_length_y); 

    // GPU might be hard to work with when the array allocated is 2D 
    int* gpuBoard; 
    int* gpuSolverBoard; 
    int* gpuMineLocations; 
    int* gpuSolverMineLocations; 
    // printf("got here inside the cuda parallel\n"); 

    // allocating memory spaces on the GPU 
    cudaMalloc(&gpuBoard, sizeof(int) * height * width); 
    cudaMalloc(&gpuSolverBoard, sizeof(int) * height * width); 
    cudaMalloc(&gpuMineLocations, sizeof(int) * 2 * mines); 
    cudaMalloc(&gpuSolverMineLocations, sizeof(int) * 2 * mines); 

    // getting information onto the GPU actually 

    board2 = (int*)malloc(height * width * sizeof(int)); 
    solverboard2 = (int*)malloc(height * width * sizeof(int)); 
    for (int i = 0; i < height; i++) { 
        for (int j = 0; j < width; j++) { 
            board2[i * width + j] = board[i][j]; 
            solverboard2[i * width + j] = solverboard[i][j]; 
        }
    }
    
    cudaMemcpy(gpuBoard, board2, sizeof(int) * height * width, cudaMemcpyHostToDevice); 
    cudaMemcpy(gpuSolverBoard, solverboard2, sizeof(int) * height * width, cudaMemcpyHostToDevice); 
    cudaMemcpy(gpuMineLocations, mineLocations, sizeof(int) * 2 * mines, cudaMemcpyHostToDevice); 
    cudaMemcpy(gpuSolverMineLocations, solverMineLocations, sizeof(int) * 2 * mines, cudaMemcpyHostToDevice); 
    
    // strange num to keep track of minesLeft 
    int* cudaCountMineFound; 
    cudaMalloc(&cudaCountMineFound, sizeof(int)); 
    cudaMemset(cudaCountMineFound, -1, sizeof(int)); 
    // for (int i = 0; i < 10; i++) { 
    //     printf("%d  ", cudaCountMineFound[i]); 
    // }
    // printf("\n"); 

    // to facilitate calling the kernel, we have to have a holder of all the changes 
    int *tempSolverBoard; 
    cudaMalloc(&tempSolverBoard, sizeof(int) * num_of_blocks * height * width); 
    // use a GPT4 generated kernel to copy from gpuSolverBoard to tempSolverBoard 
    for (int i = 0; i < num_of_blocks; i++) { 
        cudaMemcpy(tempSolverBoard + (i * height * width), gpuSolverBoard, (height * width * sizeof(int)), cudaMemcpyDeviceToDevice); 
    } 

    // printf("got here to finish some memory allocation\n"); 

    // start goinginto the main while loop 

    int* backtrace; 
    int hostbacktrace = 1; 
    cudaMalloc(&backtrace, sizeof(int)); 
    cudaMemset(backtrace, 0, sizeof(int)); 
    int minesFound = 0; 
    while (minesFound < mines) {
        // launching kernel inside 
        // second kernel launches num_of_blocks blocks and each part_side squared threads, do double tap 
        moveDoubleTap_kernel<<<num_of_blocks, threadsPerBlock>>>(gpuBoard, gpuSolverBoard, gpuMineLocations, gpuSolverMineLocations, height, width, part_length_x, part_length_y, cudaCountMineFound, tempSolverBoard); 
        cudaDeviceSynchronize(); 

        cudaMemset(backtrace, 0, sizeof(int)); 
        int temp_block = (height * width + 256 - 1)/256; 
        while (hostbacktrace == 1) { 
            update_gpu_solver_board<<<temp_block, 256>>>(gpuSolverBoard = gpuSolverBoard, tempSolverBoard = tempSolverBoard, height = height, width = width, num_of_blocks = num_of_blocks); 
            cudaDeviceSynchronize(); 
            backtracing_kernel<<<num_of_blocks, threadsPerBlock>>>(gpuBoard, gpuSolverBoard, gpuMineLocations, gpuSolverMineLocations, height, width, part_length_x, part_length_y, cudaCountMineFound, tempSolverBoard, backtrace); 
            cudaMemcpy(&hostbacktrace, backtrace, sizeof(int), cudaMemcpyDeviceToHost); 
        } 

        cudaMemcpy(&minesFound, cudaCountMineFound, sizeof(int), cudaMemcpyDeviceToHost); 
        // printf("********** %d\n", minesFound); 
        // printf("total mines: %d\n", mines); 

        // prepare for the next round 
        update_gpu_solver_board<<<temp_block, 256>>>(gpuSolverBoard = gpuSolverBoard, tempSolverBoard = tempSolverBoard, height = height, width = width, num_of_blocks = num_of_blocks); 
    } 
    minesLeft = mines - minesFound; 
    // printf("got here!!!!!\n"); 
    // cudaMemcpy(mineLocations, gpuMineLocations, sizeof(int) * 2 * mines, cudaMemcpyDeviceToHost); 
    cudaMemcpy(solverMineLocations, gpuSolverMineLocations, sizeof(int) * 2 * mines, cudaMemcpyDeviceToHost); 
    cudaMemcpy(solverboard2, gpuSolverBoard, sizeof(int) * 2 * mines, cudaMemcpyDeviceToHost); 
    // printf("got here!!!!!\n"); 

    cudaError_t errCode = cudaPeekAtLastError();
    if (errCode != cudaSuccess) {
        fprintf(stderr, "WARNING: A CUDA error occured: code=%d, %s\n", errCode, cudaGetErrorString(errCode));
    } 
    // printf("got here!!!!!\n"); 

    cudaFree(gpuBoard); 
    cudaFree(gpuSolverBoard); 
    // printf("got here!!!!!\n"); 
    cudaFree(gpuMineLocations); 
    cudaFree(gpuSolverMineLocations); 
    // printf("got here!!!!!\n"); 
}
