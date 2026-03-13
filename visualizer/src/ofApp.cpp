#include "../include/ofApp.hpp"

#include <iostream>

static const std::vector<ofColor> COLOR_AGENTS = {
    ofColor(233, 30, 99),  ofColor(33, 150, 243), ofColor(76, 175, 80),
    ofColor(255, 152, 0),  ofColor(0, 188, 212),  ofColor(156, 39, 176),
    ofColor(121, 85, 72),  ofColor(255, 187, 59), ofColor(244, 67, 54),
    ofColor(96, 125, 139), ofColor(0, 150, 136),  ofColor(63, 81, 181)};

ofApp::ofApp(Graph* _G, Plan* _P, float _agent_rad)
    : G(_G),
      P(_P),
      agent_rad(_agent_rad),
      T(P->size() - 1),
      flg_autoplay(true),
      flg_loop(true),
      flg_goal(true),
      flg_line(true)
{
}

const float scale = 10.0;

void ofApp::setup()
{
  ofBackground(ofColor(0, 0, 0));
  ofSetWindowShape(800, 800);
  ofSetCircleResolution(32);
  ofSetFrameRate(30);

  // identify size
  float x_min = std::numeric_limits<float>::max();
  float x_max = std::numeric_limits<float>::min();
  float y_min = std::numeric_limits<float>::max();
  float y_max = std::numeric_limits<float>::min();

  for (auto v : G->V) {
    x_min = std::min(x_min, v->pos.x);
    x_max = std::max(x_max, v->pos.x);
    y_min = std::min(y_min, v->pos.y);
    y_max = std::max(y_max, v->pos.y);
  }

  // setup gui
  gui.setup();
  gui.setDefaultHeight(30);
  gui.loadFont("MuseoModerno-VariableFont_wght.ttf", 15);
  gui.add(timestep_slider.setup("time step", 0, 0, T));
  gui.add(speed_slider.setup("speed", 1.0, 0, 1));

  // setup camera
  auto x_center = (x_max - x_min) / 2 + x_min;
  auto y_center = (y_max - y_min) / 2 + y_min;
  cam.setGlobalPosition(ofVec3f(x_center * scale, y_center * scale, 50));
  cam.removeAllInteractions();
  cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);
}

void ofApp::update()
{
  if (!flg_autoplay) return;

  float t = timestep_slider + speed_slider;
  if (t <= T) {
    timestep_slider = t;
  } else {
    if (flg_loop) {
      timestep_slider = 0;
    } else {
      timestep_slider = T;
    }
  }
}

void ofApp::draw()
{
  const auto z = cam.getGlobalPosition().z;
  cam.begin();

  // draw graph
  ofFill();
  ofSetColor(ofColor(100, 100, 100));
  ofSetLineWidth(1 / z);
  for (auto v : G->V) {
    ofDrawCircle(v->pos.x * scale, v->pos.y * scale, 0.2);
    for (auto u : v->neighbors) {
      ofDrawLine(v->pos.x * scale, v->pos.y * scale, u->pos.x * scale,
                 u->pos.y * scale);
    }
  }

  // draw goals
  if (flg_goal) {
    ofFill();
    const auto goal_rad = 0.1;
    const auto Q = P->at(T);
    for (auto i = 0; i < Q.size(); ++i) {
      auto color = COLOR_AGENTS[i % COLOR_AGENTS.size()];
      ofSetColor(color);
      auto pos = Q[i]->v_to->pos;
      ofDrawRectangle((pos.x - goal_rad / 2) * scale,
                      (pos.y - goal_rad / 2) * scale, goal_rad * scale,
                      goal_rad * scale);
    }
  }

  // draw agent
  ofSetLineWidth(2 / z);
  const auto Q = P->at((int)timestep_slider);
  for (auto i = 0; i < Q.size(); ++i) {
    auto color = COLOR_AGENTS[i % COLOR_AGENTS.size()];
    auto pos_from = Q[i]->v_from->pos;
    auto pos_to = Q[i]->v_to->pos;
    auto goal_pos = P->at(T)[i]->v_to->pos;
    auto step = Q[i]->step;
    auto total_step = Q[i]->total_step;

    ofFill();
    ofSetColor(color);
    ofSetLineWidth(4 / z);
    ofDrawLine(pos_from.x * scale, pos_from.y * scale, pos_to.x * scale,
               pos_to.y * scale);
    ofDrawCircle(pos_from.x * scale, pos_from.y * scale, 0.3);
    ofDrawCircle(pos_to.x * scale, pos_to.y * scale, 0.3);

    if (!(pos_from == pos_to && pos_to == goal_pos)) {
      ofFill();
      ofSetLineWidth(1 / z);
      ofSetColor(ofColor(color.r, color.g, color.b, 30));
      for (auto k = 0; k < total_step; ++k) {
        auto e = ((float)k / total_step);
        auto pos = pos_from * (1 - e) + pos_to * e;
        ofDrawCircle(pos.x * scale, pos.y * scale, agent_rad * scale);
      }
    }
    ofNoFill();
    auto e = (step + timestep_slider - timestep_slider) / total_step;
    auto pos = pos_from * (1 - e) + pos_to * e;
    ofSetColor(color);
    ofSetLineWidth(100 / z);
    ofDrawCircle(pos.x * scale, pos.y * scale, agent_rad * scale);

    if (flg_line) {
      ofSetLineWidth(1 / z);
      ofDrawLine(pos.x * scale, pos.y * scale, goal_pos.x * scale,
                 goal_pos.y * scale);
    }
  }

  cam.end();
  gui.draw();
}

void ofApp::keyPressed(int key)
{
  if (key == 'r') timestep_slider = 0;  // reset
  if (key == 'p') flg_autoplay = !flg_autoplay;
  if (key == 'l') flg_loop = !flg_loop;
  if (key == 'g') flg_goal = !flg_goal;
  if (key == 'v') flg_line = !flg_line;

  float t;
  if (key == OF_KEY_RIGHT) {
    t = timestep_slider + speed_slider;
    timestep_slider = std::min((float)T, t);
  }
  if (key == OF_KEY_LEFT) {
    t = timestep_slider - speed_slider;
    timestep_slider = std::max((float)0, t);
  }
  if (key == OF_KEY_UP) {
    t = speed_slider + 0.001;
    speed_slider = std::min(t, (float)speed_slider.getMax());
  }
  if (key == OF_KEY_DOWN) {
    t = speed_slider - 0.001;
    speed_slider = std::max(t, (float)speed_slider.getMin());
  }
}

void ofApp::keyReleased(int key) {}

void ofApp::mouseMoved(int x, int y) {}

void ofApp::mouseDragged(int x, int y, int button) {}

void ofApp::mousePressed(int x, int y, int button) {}

void ofApp::mouseReleased(int x, int y, int button) {}

void ofApp::mouseEntered(int x, int y) {}

void ofApp::mouseExited(int x, int y) {}

void ofApp::windowResized(int w, int h) {}

void ofApp::gotMessage(ofMessage msg) {}

void ofApp::dragEvent(ofDragInfo dragInfo) {}
