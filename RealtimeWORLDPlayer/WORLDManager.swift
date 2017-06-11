//
//  WORLDManager.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import AVFoundation
import Foundation
import UIKit

class WORLDManager{
    //----------i!i!i!  WORLD  i!i!i!-----------
    var world_parameter : WorldParameters? = nil
    init(wavURL:URL,wavPath:String,BufferSize:Int){
        initializeWorldParameter(wavURL: wavURL,wavPath:wavPath,BufferSize:BufferSize)
    }
    //WorldParameterの初期化
    func initializeWorldParameter(wavURL:URL,wavPath:String,BufferSize:Int){
        let Separates = wavURL.absoluteString.components(separatedBy: "///")
        world_parameter = execute_world(Separates[1],wavPath+"/output.wav",-1)
        
        Initializer(&world_parameter!,Int32(BufferSize))
    }
    func synthesisTo(buffer:AVAudioPCMBuffer,BufferSize:Int,syntheIndex:Int){
        //WORLDで合成したPCM配列を取得するための配列の定義
        let resultSynthesis_ptr = UnsafeMutablePointer<Double>.allocate(capacity:BufferSize)
        let res = Int(AddFrames(&world_parameter!,(world_parameter?.fs)!,Int32(syntheIndex),Int32(Int(BufferSize)),resultSynthesis_ptr,Int32(BufferSize),0))
        
        //もし合成に成功したら下記処理を行う。
        if (res == 1){
            //WORLDから得られた値を再生するためのbufferに入れる。
            //マルチプルバッファのため、次に再生するためのバッファに突っ込む
            for i in 0 ..< BufferSize{
                buffer.floatChannelData?.pointee[Int(i)] = Float(resultSynthesis_ptr.advanced(by: Int(i)).pointee)
            }
        }
        else{
            print("Synthesis is missed")
        }

    }
}
