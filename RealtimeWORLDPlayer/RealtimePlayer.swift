//
//  RealtimePlayer.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import Foundation
import AVFoundation

class RealtimePlayer{
    var syntheTimer:Timer!
    var bufferPlayTimer: Timer!
    var worldManager:WORLDManager? = nil
    var bufferManager:BufferManager? = nil
    var audioPlayerManager:AudioPlayerManager? = nil
    var syntheIndex:Int = 0
    var canSynthe:Bool = false
    
    init(worldManager:WORLDManager,audioFormat:AVAudioFormat){
        self.worldManager = worldManager
        self.audioPlayerManager = AudioPlayerManager(audioFormat:audioFormat)
        bufferManager = BufferManager(buffersCnt:5,bufferSize:ConstantMember.BufferSize,audioFormat: audioFormat)
        initializeTimer()
    }
    
    func initializeTimer(){
        //バッファ書き込みタイマーの設定
        bufferPlayTimer = Timer.scheduledTimer(timeInterval: 0.032, target: self, selector: #selector(self.setBufferTimer), userInfo: nil, repeats: true)
        //着火
        bufferPlayTimer.fire()
        
        //合成タイミング用タイマーの設定
        syntheTimer = Timer.scheduledTimer(timeInterval: 0.001, target: self, selector: #selector(self.synthesisTimer), userInfo: nil, repeats: true)
        //Bomb
        syntheTimer.fire()
    }
    @objc func synthesisTimer(tm: Timer){
        if canSynthe {
            worldManager?.synthesisTo(buffer: (bufferManager?.getSynthesisBuffer())!, BufferSize: (bufferManager?.bufferSize)!, syntheIndex: syntheIndex)
            canSynthe = false
        }else{
        }
    }
    @objc func setBufferTimer(tm: Timer){
        audioPlayerManager?.playBuffer(buffer: (bufferManager?.getCanWriteBuffer())!)
        canSynthe = true
        //次のバッファの設定
        bufferManager?.setNextBuffer()
    }
    func playBuffer(){
        audioPlayerManager?.playBuffer(buffer: (bufferManager?.getPlayBuffer())!)
        canSynthe = true
    }
}
