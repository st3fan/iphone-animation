/*
 * (C) Copyright 2010, Stefan Arentz, Arentz Consulting Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#import <Foundation/Foundation.h>

typedef uint32_t AnimationPixel;

// TODO: Rename this to AnimationContainerHeader
struct AnimationContainerGlobalHeader {
	uint32_t	magic;
	uint32_t	version;
	uint32_t	width;
	uint32_t	height;
	uint32_t	frameRate;
	uint32_t	frameCount;
};

typedef struct AnimationContainerGlobalHeader AnimationContainerGlobalHeader;

// TODO: Rename this to AnimationImageHeader
struct AnimationContainerImageHeader {
	uint32_t	width;
	uint32_t	height;
	uint32_t	xoffset;
	uint32_t	yoffset;
	uint32_t	format;
	uint32_t	dataLength;
};

typedef struct AnimationContainerImageHeader AnimationContainerImageHeader;

typedef enum {
	AnimationContainerImageFormatPNG = 'ping',
	AnimationContainerImageFormatUncompressedPixels = 'pixl',
	AnimationContainerImageFormatRunLengthCompressedPixels = 'rlen'
} AnimationContainerImageFormat;
