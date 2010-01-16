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
#import "AnimationCommon.h"

@interface AnimationImage : NSObject {
  @private
	const AnimationContainerImageHeader* header_;
	NSData* data_;
}

@property (nonatomic,readonly) NSUInteger width;
@property (nonatomic,readonly) NSUInteger height;
@property (nonatomic,readonly) AnimationContainerImageFormat format;
@property (nonatomic,readonly) NSData* data;

+ (id) animationImageWithImageHeader: (const AnimationContainerImageHeader*) header data: (NSData*) data;
- (id) initWithImageHeader: (const AnimationContainerImageHeader*) header data: (NSData*) data;

@end