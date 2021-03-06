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

#import <Foundation/Foundation.h>
#import "AnimationFrameBuffer.h"

@interface Animation : NSObject {
  @private
	const AnimationContainerGlobalHeader* globalHeader_;
	NSData* data_;
	NSMutableArray* images_;
}

@property (nonatomic,readonly) NSUInteger frameCount;
@property (nonatomic,readonly) NSUInteger frameRate;

+ (id) animationNamed: (NSString*) name;

- (id) initWithContentsOfFile: (NSString*) path;

- (void) drawFrame: (NSUInteger) frame intoFrameBuffer: (AnimationFrameBuffer*) buffer;

@end