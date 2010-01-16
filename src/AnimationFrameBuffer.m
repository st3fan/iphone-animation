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

#import "AnimationFrameBuffer.h"

@implementation AnimationFrameBuffer

@synthesize width = width_;
@synthesize height = height_;
@synthesize pixels = pixels_;

- (id) initWithWidth: (NSUInteger) width height: (NSUInteger) height
{
	if ((self = [super init]) != nil) {
		width_ = width;
		height_ = height;
		pixels_ = (AnimationPixel*) calloc(width * height, sizeof(AnimationPixel));
		if (pixels_ == NULL) {
			[self dealloc];
			return nil;
		}
	}
	return self;
}

- (void) dealloc
{
	if (pixels_ != NULL) {
		free(pixels_);
	}
	[super dealloc];
}

@end