#pragma once

#include "graph.hpp"
#include "ofMain.h"
#include "ofxGui.h"
#include "state.hpp"

class ofApp : public ofBaseApp
{
public:
  Graph* G;
  Plan* P;
  const float agent_rad;
  const int T;

  bool flg_autoplay;
  bool flg_loop;
  bool flg_goal;
  bool flg_line;

  // gui
  ofxFloatSlider timestep_slider;
  ofxFloatSlider speed_slider;
  ofxPanel gui;

  // camera
  ofEasyCam cam;

  void setup();
  void update();
  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  ofApp(Graph* _G, Plan* _P, float _agent_rad);
};
