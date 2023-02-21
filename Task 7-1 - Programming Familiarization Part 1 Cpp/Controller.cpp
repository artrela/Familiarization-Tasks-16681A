#include "Controller.h"
#include <cmath>
#include <iostream>
namespace mrsd
{
	void Controller::control(const mrsd::Game& g, float t)
	{
		determineSafeSpots(g);
		pickSafeSpot(g);
	}

	void Controller::createPlayer(Game& g)
	{
		if(p == 0)
		{
			p = new Player();
			p->dead = true;
		}
		if(p->dead)
		{
			p->dead = false;
			p->x = g.getWidth()/2;
			g.newPlayer(p);
		}
	}

	Prediction Controller::trackProjectile(const Projectile& p, const Game& g)
	{
		Prediction pred;

		// y = y0 +  vy0t + 0.5at^2 -> make vy = 0
		// use quadratic eq
		float t_plus = (- p.vy + std::sqrt(p.vy*p.vy - (4 * (0.5 * g.getGravity()) * p.y)) ) / (2 * (0.5 * g.getGravity()));
		float t_minus = (- p.vy - std::sqrt(p.vy*p.vy - (4 * (0.5 * g.getGravity()) * p.y)) ) / (2 * (0.5 * g.getGravity()));
		// Make sure the time is positive
		if(t_plus >= 0){pred.t = t_plus;}
		else{pred.t = t_minus;}
		
		pred.x = p.x + p.vx * pred.t;

		return pred;
	}

	std::vector<bool> Controller::determineSafeSpots(const Game& g)
	{
		// Create an array that pans the width of the screen 
		std::vector<bool> unsafe_spots(g.getWidth(),0);

		// Iterate through all projectiles, and predict their landing position & time
		std::list<Projectile> projectile_list = g.getProjectiles();

		// std::cout << projectile_list.size() << std::endl;

		std::list<Projectile>::iterator projectile;
		for(projectile = projectile_list.begin(); projectile != projectile_list.end(); projectile++){
			
			Prediction pred = trackProjectile(*projectile, g);

			// If the landing time is within 4 seconds, update put unsafe spots in safe spots
			if(pred.t < 10 && pred.x <= g.getWidth() && pred.x >= 0){

				// find the interval of unsafe spots
				float unsafe_right = pred.x + g.explosionSize * 1.5;
				float unsafe_left = pred.x - g.explosionSize * 1.5;

				// clip the explosions left and right
				if(unsafe_right > g.getWidth()){unsafe_right = g.getWidth();}
				if(unsafe_left < 0){unsafe_left = 0;}


				// update the unsafe spots 
				for(int i = std::floor(unsafe_left); i < std::ceil(unsafe_right); i++){
					// std::cout << std::floor(unsafe_left) << " " << std::ceil(unsafe_right) << std::endl;
					unsafe_spots.at(i) = 1;
				}
					
				

			}

		}	

		// obtain all the explosions
		std::list<Explosion> explosion_list = g.getExplosions();

		// Loop through all current explosions 
		std::list<Explosion>::iterator explosion;
		for(explosion = explosion_list.begin(); explosion != explosion_list.end(); explosion++){
			
			// find the interval of unsafe spots
			float unsafe_right = explosion->x + g.explosionSize;
			float unsafe_left = explosion->x - g.explosionSize;

			// clip the explosions left and right
			if(unsafe_right > g.getWidth()){unsafe_right = g.getWidth();}
			if(unsafe_left < 0){unsafe_left = 0;}


			// update the unsafe spots 
			for(int i = std::floor(unsafe_left); i < std::ceil(unsafe_right); i++){
				// std::cout << std::floor(unsafe_left) << " " << std::ceil(unsafe_right) << std::endl;
				unsafe_spots.at(i) = 1;
			}

		}

		return unsafe_spots;

	}

	int Controller::pickSafeSpot(const Game& g)
	{	
		std::vector<Player*> players_list = g.getPlayers();
		std::vector<bool> unsafe_spots = determineSafeSpots(g);

		// check if the current spot is safe
		for(int i = 0; i < players_list.size(); i++){
			
			// find the current player pos
			int player_pos = std::floor(players_list[i]->x);

			if(unsafe_spots.at(player_pos)){

				// sum the left and right windows, whichever has the higher value (more unsafe)
				// go the other direction
				int unsafe_right = 0;
				int unsafe_left = 0;

				// find & clip window if necessary
				float lim_right = g.explosionSize * 2;
				if(player_pos + g.explosionSize > g.getWidth()){

					lim_right = g.getWidth() - player_pos;
					unsafe_right = unsafe_right + g.explosionSize * 2;
				}

				float lim_left = g.explosionSize * 2;
				if(player_pos - g.explosionSize < 0){

					lim_left = player_pos - g.explosionSize;
					unsafe_left = unsafe_left + g.explosionSize * 2;

				}

				
				// find out how many unsafeties are to the right
				
				for(int i=1; i<lim_right; ++i){
					unsafe_right = unsafe_right + unsafe_spots.at(player_pos + i);
				}
				// Find how many unsafeties are left 
				
				for(int i=1; i<lim_left; ++i){
					unsafe_left = unsafe_left + unsafe_spots.at(player_pos - i);
				}

				// std::cout << unsafe_left << " " << unsafe_right << std::endl;

				// if there are more unsafeties to the right, go left
				if(unsafe_right >= unsafe_left){
					players_list[0] -> x = players_list[0] -> x - g.playerSpeed;
				}
				else{
					players_list[0] -> x = players_list[0] -> x + g.playerSpeed;
				}

			}

		}


		return 0;
	}
}
