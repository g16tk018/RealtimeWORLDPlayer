//
//  AudioManager.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import Foundation
import AVFoundation

class AudioPlayerManager{
    
    let AudioEngine = AVAudioEngine()
    var PlayerNode = AVAudioPlayerNode()
    var mixer:AVAudioMixerNode? = nil
    var audioFormat : AVAudioFormat? = nil
    
    init(audioFormat:AVAudioFormat){
        self.audioFormat = audioFormat
        initializeAudio()
    }
    func initializeAudio(){
        mixer = AudioEngine.mainMixerNode
        AudioEngine.attach(PlayerNode)
        AudioEngine.connect(PlayerNode,to: mixer!, format:audioFormat)
        AudioEngine.prepare()
        try! AudioEngine.start()
        PlayerNode.play()
    }
    func playBuffer(buffer:AVAudioPCMBuffer){
        PlayerNode.scheduleBuffer(buffer,at:nil,completionHandler:nil)
    }
}
