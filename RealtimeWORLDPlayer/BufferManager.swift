//
//  BufferManager.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import Foundation
import AVFoundation

class BufferManager{
    var bufferCnt:Int = 0
    var bufferSize:Int = 0
    var audioFormat:AVAudioFormat? = nil
    var buffers:[AVAudioPCMBuffer] = []
    var bufferNum:Int = 0
    
    init(buffersCnt:Int,bufferSize:Int,audioFormat:AVAudioFormat){
        self.bufferCnt = buffersCnt
        self.bufferSize = bufferSize
        self.audioFormat = audioFormat
        initializeBuffer()
    }
    func bufferCleaner(){
        for i in 0..<bufferCnt{
            for j in 0..<bufferSize{
                buffers[i].floatChannelData?.pointee[j] = 0.0
            }
        }
    }
    func initializeBuffer(){
        for _ in 0 ..< bufferCnt{
            let buff:AVAudioPCMBuffer = AVAudioPCMBuffer(pcmFormat:audioFormat!,frameCapacity:AVAudioFrameCount(bufferSize))
            buff.frameLength = AVAudioFrameCount(bufferSize)
            buffers.append(buff)
        }
    }
    func getSynthesisBuffer()->AVAudioPCMBuffer{
        return buffers[(bufferNum+2)%bufferCnt]
    }
    func getCanWriteBuffer()->AVAudioPCMBuffer{
        return buffers[(bufferNum+1)%bufferCnt]
    }
    func getPlayBuffer()->AVAudioPCMBuffer{
        let res = buffers[bufferNum]
        return res
    }
    func setNextBuffer(){
        bufferNum = (bufferNum+1)%bufferCnt
    }
}
