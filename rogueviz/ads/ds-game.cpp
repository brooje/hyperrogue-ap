// Relative Hell: implemetation of the de Sitter game
// Copyright (C) 2022-2025 Zeno Rogue, see '../../hyper.cpp' for details

namespace hr {

namespace ads_game {

void print(hstream& hs, cross_result cr) { print(hs, cr.h, "@", cr.shift); }

void init_textures();
void draw_textures();

vector<ld> shape_disk;

void set_default_keys();

/* In DS, we also use ads_matrix, but the meaning of the shift parameter is different:
 * 
 */

vector<unique_ptr<ads_object>> rocks;

bool disable_ds_gen;

struct rock_generator {
  ld cshift;

  ads_object* add(transmatrix T) {
    auto r = std::make_unique<ads_object> (oRock, nullptr, ads_matrix(T, cshift), 0xFFFFFFFF);
    r->shape = &shape_disk; r->resource = rtNone;
    auto res = &*r;
    rocks.emplace_back(std::move(r));
    return res;
    };

  void report(string s) {
    println(hlog, lalign(10, hr::format(tformat, cshift/ds_time_unit)), ": ", s);
    };

  ld rand_range(ld a, ld b) { return lerp(a, b, randd()); };

  transmatrix rand_place() {
    geometry = gSphere;
    hyperpoint h = random_spin3() * C0;
    transmatrix T = gpushxto0(h);
    geometry = gSpace435;
    for(int i=0; i<4; i++) T[i][3] = T[3][i] = i == 3;
    return T;
    };

  void death_cross(int qty) {
    ld rapidity = rand_range(1, 3);
    cshift += rand_range(0.5, 1);
    ld alpha = randd() * TAU;
    report(lalign(0, "Death Cross ", qty));
    for(int a=0; a<qty; a++)
      add(spin(a * TAU / qty + alpha) * lorentz(0, 3, rapidity));
    cshift += rand_range(0.5, 1);
    }

  void static_starry_field() {
    cshift += rand_range(1, 2);
    report("Static Starry Field");
    for(int i=0; i<100; i++) {
      transmatrix T = rand_place();
      add(inverse(T));
      }
    cshift += rand_range(1, 2);
    }

  void chaotic_starry_field() {
    cshift += rand_range(2, 3);
    report("Chaotic Starry Field");
    for(int i=0; i<50; i++) {
      transmatrix T = rand_place();
      add(inverse(T) * spin(randd() * TAU) * lorentz(0, 3, rand_range(0, 3)));
      }
    cshift += rand_range(2, 3);
    }

  /* that pattern does not work */
  void death_spiral() {
    cshift += rand_range(2, 3) + 1.5;
    report("Death Spiral");
    for(int i=0; i<30; i++) {
      add(spin(i * TAU * 14 / 30) * lorentz(0, 3, exp((i-15)/5.)));
      }
    cshift += rand_range(2, 3);
    }

  transmatrix div_matrix() {
    /* we need to find the limit of this as appr -> inf */
    ld appr = 5;
    transmatrix T = lorentz(2, 3, -appr) * cspin(0, 2, exp(-appr)) * lorentz(2, 3, appr);
    /* all the entries happen to be multiples of .125 */
    for(int i=0; i<4; i++) for(int j=0; j<4; j++) {
      auto& b = T[i][j];
      b = floor(b * 8 + .5) / 8;
      }
    return T;
    }

  /* see div_matrix */
  transmatrix conv_matrix() {
    ld appr = 5;
    transmatrix T = lorentz(2, 3, appr) * cspin(0, 2, exp(-appr)) * lorentz(2, 3, -appr);
    for(int i=0; i<4; i++) for(int j=0; j<4; j++) {
      auto& b = T[i][j];
      b = floor(b * 8 + .5) / 8;
      }
    return T;
    }

  void divergent_spiral() {
    report("Divergent Spiral");
    cshift += rand_range(.3, .7);
    ld alpha = randd() * TAU;
    ld step = rand_range(0.17, 0.23);
    for(int i=0; i<45; i++) {
      cshift += step;
      add(spin(alpha + i * TAU / 30) * div_matrix())->subtype = 1;
      }
    cshift += rand_range(.3, .7);
    }

  void convergent_spiral() {
    report("Convergent Spiral");
    cshift += rand_range(.3, .7);
    ld alpha = randd() * TAU;
    ld step = rand_range(0.17, 0.23);
    for(int i=0; i<45; i++) {
      cshift += step;
      add(spin(alpha + i * TAU / 30) * conv_matrix())->subtype = 2;
      }
    cshift += rand_range(.3, .7);
    }

  void rack() {
    report("Rack");
    ld ds_scale = get_scale();
    int qty = 3 + rand() % 4;
    ld rapidity = rand_range(1, 3);
    ld step = rand_range(.45, .75) * ds_scale;
    ld alpha = rand_range(0, TAU);
    ld spinv = rand_range(0, TAU);
    for(int i=0; i<qty; i++) {
      cshift ++;
      for(ld j=-3; j<=3; j++) {
        add(spin(alpha + i * spinv) * cspin(0, 2, j * step) * spin90() * lorentz(0, 3, rapidity));
        }
      }
    }

  void hyperboloid() {
    report("Hyperboloid");
    ld ds_scale = get_scale();
    ld alpha = randd() * TAU;
    ld range1 = rand_range(0.15, 0.25) * ds_scale;
    ld range2 = rand_range(0.35, 0.45) * ds_scale;
    cshift += rand_range(2, 3);
    ld rapidity = rand_range(-3, 3);
    int qty = 20 + rand() % 10;
    for(int i=0; i<qty; i++)
      add(spin(alpha) * cspin(0, 2, range1) * spin(i * TAU / qty) * cspin(0, 2, range2) * lorentz(1, 3, rapidity));
    cshift += rand_range(2, 3);
    }

  void machinegun() {
    report("Machinegun");
    ld alpha = randd() * TAU;
    int qty = 10 + 1 / (.05 + randd());
    ld rapidity = rand_range(3, 6);
    ld step = rand_range(0.1, 0.15);
    for(int i=0; i<qty; i++) {
      cshift += step;
      add(spin(alpha) * lorentz(1, 3, rapidity));
      }
    }

  void add_random() {

    int r = rand() % 150;

    #define Chance(q) if(r < 0) return; r -= (q); if(r < 0)
    Chance(10) death_cross(4);
    Chance(10) death_cross(3);
    Chance(10) static_starry_field();
    Chance(10) chaotic_starry_field();
    Chance(10) divergent_spiral();
    Chance(10) convergent_spiral();
    Chance(10) rack();
    Chance(10) hyperboloid();
    Chance(10) machinegun();
    #undef Chance
    }

  void add_until(ld t) {
    if(disable_ds_gen) return;
    while(cshift < t) add_random();
    }

  void add_rsrc_until(ld t) {
    if(disable_ds_gen) return;
    while(cshift < t) {
      ld rapidity = rand_range(0, 3);
      ld step = rand_range(.2, .5);
      ld alpha = rand_range(0, TAU);
      cshift += rand_range(0.5, 1) * (1 + cshift / 10);
      auto r = add(spin(alpha) * cspin(0, 2, step) * spin90() * lorentz(0, 3, rapidity));
      eResourceType rt = eResourceType(1 + rand() % 4);
      r->type = oResource;
      r->resource = rt;
      r->shape = rsrc_shape[rt];
      r->col = rsrc_color[rt];
      }
    }

  };

rock_generator rockgen, rsrcgen;

auto future_shown = 5 * TAU;

auto future_shown_condiv = 2 * TAU;

/** start with a fixed good-looking sequence */
bool demo;

void init_ds_game() {

  dynamicval<eGeometry> g(geometry, gSpace435);

  rockgen.cshift = 0;
  rsrcgen.cshift = 0;

  /* create the main rock first */
  main_rock = rockgen.add(Id);
  main_rock->col = 0xFFD500FF;
  main_rock->type = oMainRock;

  main_rock = rockgen.add(Id);
  main_rock->col = 0xFF;
  main_rock->shape = &shape_gold;
  main_rock->type = oMainRock;

  // do not shift time
  main_rock->pt_main.shift = 0;

  /* also create shape_disk */
  shape_disk.clear();
  for(int d=0; d<=360; d += 15) {
    shape_disk.push_back(sin(d*degree) * 0.1);
    shape_disk.push_back(cos(d*degree) * 0.1);
    }

  rockgen.cshift += 2;
  if(demo) {
    rockgen.static_starry_field();
    rockgen.hyperboloid();
    rockgen.chaotic_starry_field();
    rockgen.rack();
    }
  rockgen.add_until(future_shown);
  
  rsrcgen.cshift += 1;
  rsrcgen.add_rsrc_until(future_shown);

  }

void ds_gen_particles(int qty, transmatrix from, ld shift, color_t col, ld spd, ld t, ld spread = 1) {
  for(int i=0; i<qty; i++) {
    auto r = std::make_unique<ads_object>(oParticle, nullptr, ads_matrix(from * spin(randd() * TAU * spread) * lorentz(0, 3, (.5 + randd() * .5) * spd), shift), col );
    r->shape = &shape_particle;
    r->life_end = randd() * t;
    r->life_start = 0;
    rocks.emplace_back(std::move(r));
    }
  }

void ds_crash_ship(const string &reason) {
  if(ship_pt < invincibility_pt) return;
  common_crash_ship(reason);
  dynamicval<eGeometry> g(geometry, gSpace435);
  ds_gen_particles(rpoisson(crash_particle_qty * 2), inverse(current.T) * spin(ang*degree), current.shift, rsrc_color[rtHull], crash_particle_rapidity, crash_particle_life);
  }

void ds_handle_crashes() {
  if(paused) return;
  dynamicval<eGeometry> g(geometry, gSphere);
  vector<ads_object*> dmissiles;
  vector<ads_object*> drocks;
  vector<ads_object*> dresources;
  for(auto m: displayed) {
    if(m->type == oMissile)
      dmissiles.push_back(m);
    if(m->type == oRock || m->type == oMainRock)
      drocks.push_back(m);
    if(m->type == oResource)
      dresources.push_back(m);
    }

  for(auto m: dmissiles) {
    hyperpoint h = kleinize(m->pt_main.h);
    for(auto r: drocks) {
      if(pointcrash(h, r->pts)) {
        m->life_end = m->pt_main.shift;
        cur.rocks_hit++;
        if(r->type != oMainRock)
          r->life_end = r->pt_main.shift;
        dynamicval<eGeometry> g(geometry, gSpace435);
        ds_gen_particles(rpoisson(crash_particle_qty), m->at.T * lorentz(2, 3, m->life_end), m->at.shift, missile_color, crash_particle_rapidity, crash_particle_life);
        if(r->type != oMainRock)
          ds_gen_particles(rpoisson(crash_particle_qty), r->at.T * lorentz(2, 3, r->life_end), r->at.shift, r->col, crash_particle_rapidity, crash_particle_life);
        playSound(nullptr, "hit-crush3");
        break;
        }
      }
    }

  if(!game_over) for(int i=0; i<isize(shape_ship); i+=2) {
    ld ds_scale = get_scale();
    hyperpoint h = spin(ang*degree) * hpxyz(shape_ship[i] * ds_scale, shape_ship[i+1] * ds_scale, 1);
    for(auto r: drocks) {
      if(pointcrash(h, r->pts)) ds_crash_ship(r == main_rock ? "crashed into the home star" : "crashed into a star");
      }
    for(auto r: dresources) {
      if(pointcrash(h, r->pts)) {
        r->life_end = r->pt_main.shift;
        cur.rsrc_collected++;
        gain_resource(r->resource);
        }
      }
    }
  }

void ds_fire() {
  if(!pdata.ammo) return;
  pdata.ammo--;
  playSound(nullptr, "fire");
  dynamicval<eGeometry> g(geometry, gSpace435);

  transmatrix S0 = inverse(current.T) * spin(ang*degree);

  transmatrix S1 = S0 * lorentz(0, 3, ads_missile_rapidity);

  auto r = std::make_unique<ads_object> (oMissile, nullptr, ads_matrix(S1, current.shift), rsrc_color[rtAmmo]);
  r->shape = &shape_missile;
  r->life_start = 0;

  rocks.emplace_back(std::move(r));
  }

bool have_crashes = true;

bool ds_turn(int idelta) {
  multi::handleInput(idelta, multi::scfg_default);
  ld delta = idelta / 1000.;
  
  if(!(cmode & sm::NORMAL)) return false;

  if(have_crashes) ds_handle_crashes();
  if(no_param_change && !all_params_default()) no_param_change = false;

  auto& act = multi::action_states[1];

  if(act[multi::pcFire].pressed() && !paused && !game_over) ds_fire();
  if(act[pcPause].pressed()) switch_pause();
  if(act[pcDisplayTimes].pressed()) view_proper_times = !view_proper_times;
  if(act[pcSwitchSpin].pressed()) auto_rotate = !auto_rotate;
  if(act[pcMenu].pressed()) pushScreen(game_menu);
  
  if(true) {
    dynamicval<eGeometry> g(geometry, gSpace435);
    
    ld pt = delta * ds_simspeed;
    ld mul = read_movement();
    ld dv = pt * ds_accel * mul;

    if(paused && act[pcPauseMoveSwitch]) {
      current.T = spin(ang*degree) * cspin(0, 2, mul*delta*-pause_speed) * spin(-ang*degree) * current.T;
      }
    else {
      current.T = spin(ang*degree) * lorentz(0, 3, -dv) * spin(-ang*degree) * current.T;
      }
    
    if(!paused) {
      ld ds_scale = get_scale();
      pdata.fuel -= dv;
      ds_gen_particles(rpoisson(dv*fuel_particle_qty), inverse(current.T) * spin(ang*degree+M_PI) * twist::uxpush(0.06 * ds_scale), current.shift, rsrc_color[rtFuel], fuel_particle_rapidity, fuel_particle_life, 0.02);
      }

    ld tc = 0;
    if(!paused) tc = pt;
    else if(act[pcPauseFuture]) tc = pt;
    else if(act[pcPausePast]) tc = -pt;

    if(!paused && !game_over) {
      shipstate ss;
      ss.at.T = inverse(current.T) * spin(ang*degree);
      ss.at.shift = current.shift;
      ss.start = ship_pt;
      ss.current = current;
      ss.duration = pt;
      ss.ang = ang;
      history.emplace_back(ss);
      }
    
    current.T = lorentz(3, 2, -tc) * current.T;

    auto& mshift = main_rock->pt_main.shift;
    if(mshift && !isnan(mshift)) {
      #if RVCOL
      constexpr ld win_time = 60;
      if(pdata.score[0] < win_time && (current.shift + mshift) >= win_time && !game_over && no_param_change)
        rogueviz::rv_achievement("DSGAME");
      #endif
      current.shift += mshift;
      current.T = current.T * lorentz(2, 3, mshift);
      mshift = 0;
      pdata.score[0] = max(pdata.score[0], current.shift);
      }
    fixmatrix(current.T);
    
    if(1) {
      rockgen.add_until(current.shift + future_shown);
      rsrcgen.add_rsrc_until(current.shift + future_shown);
      }

    if(!paused) {
      ship_pt += pt;
      pdata.oxygen -= pt;
      if(pdata.oxygen < 0) {
        pdata.oxygen = 0;
        game_over_with_message("suffocated");
        }
      }
    else view_pt += tc;

    if(act[multi::pcFire].pressed()&& false) {
      if(history.size())
        history.back().duration = HUGE_VAL;
      current = random_spin3();
      }
    }

  return true;
  }

hyperpoint pov = point30(0, 0, 1);

cross_result ds_cross0_cone(const transmatrix& T, ld which) {

  ld a = T[2][2];
  ld b = T[2][3];
  // a * cosh(t) + b * sinh(t) = 1
  // solution: t = log((1 +- sqrt(-a^2 + b^2 + 1))/(a + b))

  ld underroot = (1+b*b-a*a);
  if(underroot < 1e-10) underroot = 0;
  if(underroot < 0) return cross_result { Hypc, 0};

  ld underlog = (1 + which * sqrt(underroot)) / (a + b);
  if(underlog < 0) return cross_result { Hypc, 0};

  ld t = log(underlog);

  cross_result res;
  res.shift = t;
  res.h = T * hyperpoint(0, 0, cosh(t), sinh(t));

  if(abs(res.h[2] - 1) > .01) return cross_result{Hypc, 0};

  res.h /= hypot_d(3, res.h);
  res.h[3] = 0;

  // res.h[2] = sqrt(1 - res.h[3] * res.h[3]); res.h[3] = 0;

  return res;
  }

cross_result ds_cross0_sim(const transmatrix& T) {
  // h = T * (0 0 cosh(t) sinh(t))
  // T[3][2] * cosh(t) + T[3][3] * sinh(t) = 0
  // T[3][2] + T[3][3] * tanh(t) = 0
  ld tt = - T[3][2] / T[3][3];
  if(tt < -1 || tt > 1) return cross_result{ Hypc, 0 };
  cross_result res;
  ld t = atanh(tt);
  res.shift = t;
  res.h = T * hyperpoint(0, 0, cosh(t), sinh(t));
  return res;
  }

cross_result ds_cross0(const transmatrix& T) {
  return which_cross ? ds_cross0_cone(T, which_cross) : ds_cross0_sim(T);
  }

cross_result ds_cross0_light(transmatrix T) {
  // h = T * (t 0 1 t); h[3] == 0
  ld t = T[3][2] / -(T[3][0] + T[3][3]);
  cross_result res;
  res.shift = t;
  res.h = T * hyperpoint(t, 0, 1, t);
  return res;
  }

transmatrix tpt_scaled(ld x, ld y) {
  return cspin(0, 2, x) * cspin(1, 2, y);
  }

transmatrix tpt(ld x, ld y) {
  ld ds_scale = get_scale();
  return tpt_scaled(x * ds_scale, y * ds_scale);
  }

// sometimes the result may be incorrect due to numerical precision -- don't show that then in this case
bool invalid(cross_result& res) {
  ld val = sqhypot_d(3, res.h);
  if(abs(val-1) > 1e-3 || isnan(val) || abs(res.h[3]) > 1e-3 || isnan(res.h[3])) return true;
  return false;
  }

void view_ds_game() {
  displayed.clear();
  mousetester = kleinize(unshift(mouseh));
  under_mouse.clear();

  bool hv = hyperbolic;
  bool hvrel = among(pmodel, mdRelPerspective, mdRelOrthogonal);

  sphereflip = hv ? Id : sphere_flipped ? MirrorZ : Id;

  copyright_shown = "";
  if(!hv) draw_textures();

  bool only_main = false;

  if(1) {
    for(auto& r: rocks) {
      auto& rock = *r;
      poly_outline = 0xFF;
      if(rock.type == oMainRock) rock.at.shift = current.shift;
    
      if(current.shift < rock.at.shift - (rock.subtype == 1 ? future_shown_condiv : future_shown)) continue;
      if(current.shift > rock.at.shift + (rock.subtype == 2 ? future_shown_condiv : future_shown)) continue;

      if(1) {
        dynamicval<eGeometry> g(geometry, gSpace435);
        transmatrix at = current.T * lorentz(2, 3, rock.at.shift - current.shift) * rock.at.T;
        rock.pt_main = ds_cross0(at);
        
        if(invalid(rock.pt_main)) continue;

        if(rock.pt_main.shift < rock.life_start) continue;
        if(rock.pt_main.shift > rock.life_end) continue;

        transmatrix at1 = at * lorentz(2, 3, rock.pt_main.shift);
        rock.pts.clear();
        
        auto& sh = *rock.shape;

        bool bad = false;
        for(int i=0; i<isize(sh); i+=2) {
          transmatrix at2 = at1 * tpt(sh[i], sh[i+1]);
          auto cr1 = ds_cross0(at2);
          if(invalid(cr1)) { bad = true; continue; }
          rock.pts.push_back(cr1);
          }
        if(bad) continue;
        }
      
      vector<hyperpoint> circle_flat;
      for(auto c: rock.pts) circle_flat.push_back(c.h / (1 + c.h[2]));

      if(rock.type != oParticle && pointcrash(mousetester, rock.pts)) {
        if(only_main) break;
        if(&rock == main_rock) { under_mouse.clear(); only_main = true; }
        under_mouse.push_back(&rock);
        }
      
      ld area = 0;
      for(int i=0; i<isize(circle_flat)-1; i++)
        area += (circle_flat[i] ^ circle_flat[i+1]) [2];
      area += (circle_flat.back() ^ circle_flat[0]) [2];

      if(area > 0) continue;

      if(hv) {
        ld t = rock.at.shift;
        if(rock.type == oMainRock) t = floor(t / spacetime_step + .5) * spacetime_step;
        transmatrix at = current.T * lorentz(2, 3, t - current.shift) * rock.at.T;
        for(int z0=-spacetime_qty; z0<=spacetime_qty; z0++) {
          ld z = z0 * spacetime_step;
          if(t-z < rock.life_start) continue;
          if(t-z > rock.life_end) continue;
          transmatrix at1 = at * lorentz(2, 3, z);
          if((at1 * pov) [2] < 0) continue;

          auto& sh = *rock.shape;

          for(int i=0; i<isize(sh); i+=2) {
            hyperpoint h = hvrel ? tpt(sh[i], sh[i+1]) * pov: hpxy(sh[i], sh[i+1]);
            curvepoint(h);
            }
          curvepoint_first();
          color_t col = rock.col;
          if(col == 0xFF) col = 0xFFD500FF;
          if(col != 0xFFD500FF && !hvrel) part(col, 0) = part(col, 0) / 4;
          queuecurve(shiftless(at1), col, 0, PPR::TRANSPARENT_WALL);
          }
        }

      if(!hv) {
        for(auto p: rock.pts) curvepoint(p.h);
        queuecurve(shiftless(sphereflip), rock.col, rock.col, obj_prio[rock.type]);
        }

      if(pmodel == mdPerspective) {
        for(auto p: rock.pts) curvepoint(p.h);
        curvepoint_first();
        color_t col = rock.col; part(col, 0) /= 2;
        queuecurve(shiftless(sphereflip), ghost_color, 0, obj_prio[rock.type]).flags |= POLY_NO_FOG | POLY_FORCEWIDE;
        }

      if(rock.type != oParticle) {
        ld t = rock.pt_main.shift;
        if(rock.type == oMainRock) t += current.shift;
        view_time(shiftless(sphereflip * rgpushxto0(rock.pt_main.h)), t, 0xFFFF00);
        }

      if(rock.pt_main.h[2] > 0.1 && rock.life_end == HUGE_VAL) {
        displayed.push_back(&rock);
        }
      }      

    ld delta = paused ? 1e-4 : -1e-4;
    ld last_shown = -100;
    vector<ld> times;
    if(paused) for(auto& ss: history) {
      if(ss.start < last_shown + ship_history_period) continue;
      last_shown = ss.start; times.push_back(ss.start);
      if(ss.at.shift < current.shift - 4 * TAU) continue;
      if(ss.at.shift > current.shift + 4 * TAU) continue;

      if(hv) render_ship_parts([&] (const hpcshape& sh, color_t col, int sym) {
        int dx = sym ? -1 : 1;        
        for(int i=sh.s; i<sh.e; i ++) {
          auto x = cgi.hpc[i][0];
          auto y = cgi.hpc[i][1]*dx;
          hyperpoint h = hvrel ? tpt_scaled(x, y) * pov: hpxy(x, y);
          curvepoint(h);
          }
        curvepoint_first();
        shiftmatrix S = shiftless(current.T * lorentz(2, 3, ss.at.shift - current.shift) * ss.at.T);
        queuecurve(S, col, 0, PPR::TRANSPARENT_WALL);
        });
       
      dynamicval<eGeometry> g(geometry, gSpace435);
      cross_result cr = ds_cross0(current.T * lorentz(2, 3, ss.at.shift - current.shift) * ss.at.T);
      if(cr.shift < delta) continue;
      if(cr.shift > ss.duration + delta) continue;
      transmatrix at = current.T * lorentz(2, 3, cr.shift) * ss.at.T;
      
      render_ship_parts([&] (const hpcshape& sh, color_t col, int sym) {
        geometry = gSpace435;
        vector<hyperpoint> pts;
        int dx = sym ? -1 : 1;
        
        for(int i=sh.s; i<sh.e; i ++) {
          transmatrix at1 = at * tpt_scaled(cgi.hpc[i][0], cgi.hpc[i][1]*dx);
          pts.push_back(ds_cross0(at1).h);
          }
        
        geometry = g.backup;

        if(!hv) {
          for(auto pt: pts) curvepoint(pt);
          queuecurve(shiftless(sphereflip), 0xFF, col, PPR::MONSTER_FOOT);
          }

        if(pmodel == mdPerspective) {
          if(col == shipcolor) col = ghost_color;
          for(auto pt: pts) curvepoint(pt);
          queuecurve(shiftless(sphereflip), col, 0, PPR::MONSTER_FOOT).flags |= POLY_NO_FOG | POLY_FORCEWIDE;
          }
        });

      view_time(shiftless(sphereflip * rgpushxto0(cr.h)), cr.shift + ss.start, 0xC0C0C0);
      }

    if(!game_over && !paused) {
      poly_outline = 0xFF;
      if(ship_pt < invincibility_pt && invincibility_pt < HUGE_VAL) {
        ld u = (invincibility_pt-ship_pt) / ds_how_much_invincibility;
        poly_outline = gradient(shipcolor, rsrc_color[rtHull], 1, cos(5*u*TAU), -1);
        }
      render_ship_parts([&] (const hpcshape& sh, color_t col, int sym) {
        if(hv) {
          int dx = sym ? -1 : 1;
          for(int i=sh.s; i<sh.e; i++) {
            transmatrix at1 = tpt_scaled(cgi.hpc[i][0], cgi.hpc[i][1] * dx);
            curvepoint(ds_cross0(at1).h);
            }
          queuecurve(shiftless(sphereflip * spin(ang*degree)), col, 0, PPR::MONSTER_HAIR).flags |= POLY_NO_FOG | POLY_FORCEWIDE;
          }
        else {
          shiftmatrix M = shiftless(sphereflip * spin(ang*degree));
          if(sym) M = M * MirrorY;
          queuepolyat(M, sh, col, PPR::MONSTER_HAIR);
          }
        });
      poly_outline = 0xFF;

      view_time(shiftless(sphereflip), ship_pt, 0xFFFFFF);
      }
    
    if(paused && !game_over && !in_replay && !hv && !which_cross) {
      vector<hyperpoint> pts;
      int ok = 0, bad = 0;
      for(int i=0; i<=360; i++) {
        dynamicval<eGeometry> g(geometry, gSpace435);
        auto h = inverse(current_ship.T) * spin(i*degree);
        auto cr = ds_cross0_light(current.T * lorentz(2, 3, current_ship.shift - current.shift) * h);
        pts.push_back(cr.h);
        if(cr.shift > 0) ok++; else bad++;
        }
      for(auto h: pts) curvepoint(h);
      queuecurve(shiftless(sphereflip), 0xFF0000C0, bad == 0 ? 0x00000060 : 0xFFFFFF10, PPR::SUPERLINE);
      }
    }

  view_footer();
  }

void ds_restart() {

  if(in_spacetime()) {
    switch_spacetime();
    ds_restart();
    switch_spacetime();
    return;
    }

  main_rock = nullptr;

  if(true) {
    ld ds_scale = get_scale();
    dynamicval<eGeometry> g(geometry, gSpace435);
    current = cspin(0, 2, 0.2 * ds_scale);
    invincibility_pt = ds_how_much_invincibility;
    }

  ship_pt = 0;
  no_param_change = all_params_default();

  rocks.clear();
  history.clear();
  displayed.clear();
  init_ds_game();
  reset_textures();
  pick_textures();
  init_rsrc();
  init_gamedata();
  in_replay = false;
  }

void run_ds_game_hooks() {
  rogueviz::rv_hook(hooks_frame, 100, view_ds_game);
  rogueviz::rv_hook(shmup::hooks_turn, 0, ds_turn);
  rogueviz::rv_hook(hooks_prestats, 100, display_rsrc);
  rogueviz::rv_hook(hooks_handleKey, 150, handleKey);
  rogueviz::rv_hook(anims::hooks_anim, 100, replay_animation);
  rogueviz::rv_hook(hooks_global_mouseover, 100, generate_mouseovers);
  rogueviz::rv_change<color_t>(titlecolor, 0xFFC000);
  rv_hook(hooks_music, 100, [] (eLand& l) { l = mfcode("C5"); return false; });
  }

void run_ds_game() {

  stop_game();
  run_size_hooks();
  set_geometry(gSphere);
  start_game();

  init_textures();
  pick_textures();

  ds_restart();

  run_ds_game_hooks();
  }

void add_ds_cleanup() {
  rogueviz::on_cleanup_or_next([] {
    main_rock = nullptr;
    displayed.clear();
    });
  }

void run_ds_game_std() {
  lps_enable(&lps_relhell_space);
  enable_canvas();
  run_ds_game();
  }

auto ds_hooks = 
  arg::add3("-ds-game1", run_ds_game) +
  arg::add3("-ds-game", run_ds_game_std);

}
}
