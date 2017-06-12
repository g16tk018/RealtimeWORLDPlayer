//
//  ViewController.swift
//  VowelsMorphing
//
//  Created by Yusuke Watanabe on 2017/06/11.
//  Copyright © 2017年 Yusuke Watanabe. All rights reserved.
//

import UIKit
import SpriteKit
import AVFoundation

class ViewController: UIViewController {
    var worldManager:WORLDManager? = nil
    var realtimePlayer:RealtimePlayer? = nil
    var spriteView:SKView?
    var spriteScene:SKScene?
    let AudioFormat = AVAudioFormat(standardFormatWithSampleRate: 16000, channels: AVAudioChannelCount(2))
    
    let wavPath = NSSearchPathForDirectoriesInDomains(.documentDirectory,.userDomainMask,true)[0] as String
    
    override func viewDidLoad() {
        super.viewDidLoad()
        initializeView()
        
        let wavURL = URL(fileURLWithPath: Bundle.main.path(forResource: "vaiueo2d", ofType: "wav",inDirectory:"waves")!)
        worldManager = WORLDManager(wavURL:wavURL,wavPath:wavPath,BufferSize:ConstantMember.BufferSize)
        
        realtimePlayer = RealtimePlayer(worldManager: self.worldManager!,audioFormat: AudioFormat)
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
    }
    //画面上の初期化
    func initializeView(){
        spriteView = SKView(frame:CGRect(x: 0.0, y: 0.0, width: self.view.frame.width, height: self.view.frame.height))
        self.spriteScene = SKScene(size:CGSize(width:(spriteView?.frame.width)!,height:(spriteView?.frame.height)!))
        spriteView!.presentScene(spriteScene)
        
        self.view.addSubview(spriteView!)
    }
    //タップ開始
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch: AnyObject in touches{
            let Location = touch.location(in:self.view)
            let ViewLocation = CGPoint(x:Location.x,y:self.view.frame.height - Location.y)
            realtimePlayer?.syntheIndex = getSyntheIndexFrom(location: ViewLocation)
        }
    }
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch: AnyObject in touches{
            let Location = touch.location(in:self.view)
            let ViewLocation = CGPoint(x:Location.x,y:self.view.frame.height - Location.y)
            realtimePlayer?.syntheIndex = getSyntheIndexFrom(location: ViewLocation)
        }
    }
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        realtimePlayer?.syntheIndex = 0
        realtimePlayer?.canSynthe = false
    }
    func getSyntheIndexFrom(location:CGPoint)->Int{
        let xAxisMargin = self.view.frame.width/CGFloat((worldManager?.world_parameter!.f0_length)!)
        return Int(location.x / xAxisMargin)
    }
}

