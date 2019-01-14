/**********************************************************************************
  This file is part of the game 'KTron'

  Copyright (C) 1998-2000 by Matthias Kiefer <matthias.kiefer@gmx.de>
  Copyright (C) 2005 Benjamin C. Meyer <ben at meyerhome dot net>
  Copyright (C) 2008-2009 Stas Verberkt <legolas at legolasweb dot nl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  *******************************************************************************/
  
#include "intelligence.h"

#include "tron.h"
#include "settings.h"

#include <KgDifficulty>

Intelligence::Intelligence()
{
	m_random.setSeed(0);

	m_lookForward = 15;
}

void Intelligence::referenceTron(Tron *t)
{
	m_tron = t;
}

//
// Settings
//

/** retrieves the opponentSkill */
int Intelligence::opponentSkill()
{
	switch (Kg::difficultyLevel())
	{
		case KgDifficultyLevel::VeryEasy:
			return 1;
		default:
		case KgDifficultyLevel::Easy:
			return 2;
		case KgDifficultyLevel::Medium:
			return 3;
		case KgDifficultyLevel::Hard:
			return 4;
		case KgDifficultyLevel::VeryHard:
			return 5;
	}
}

//
// Algorithm helper function
//

void Intelligence::changeDirection(int playerNr,int dis_right,int dis_left)
{
   PlayerDirections::Direction currentDir = m_tron->getPlayer(playerNr)->getDirection();
   PlayerDirections::Direction sides[2];
   sides[0] = PlayerDirections::None;
   sides[1] = PlayerDirections::None;
   
   switch (currentDir)
   {
  		case PlayerDirections::Left:
    		//turns to either side
    		sides[0] = PlayerDirections::Down;
    		sides[1] = PlayerDirections::Up;
    		break;
  		case PlayerDirections::Right:
    		sides[0] = PlayerDirections::Up;
    		sides[1] = PlayerDirections::Down;
    		break;
  		case PlayerDirections::Up:
    		sides[0] = PlayerDirections::Left;
    		sides[1] = PlayerDirections::Right;
    		break;
  		case PlayerDirections::Down:
    		sides[0] = PlayerDirections::Right;
    		sides[1] = PlayerDirections::Left;
    		break;
		default:
		break;

  	}

   if(!(dis_left == 1 && dis_right == 1))
   {
			// change direction
			if ((int)m_random.getLong(100) <= (100*dis_left)/(dis_left+dis_right))
			{
	  			if (dis_left != 1)
		    		// turn to the left
		    		m_tron->getPlayer(playerNr)->setDirection(sides[0]);
	  			else
	   	 		// turn to the right
	    			m_tron->getPlayer(playerNr)->setDirection(sides[1]);
	    	}
			else
			{
	  				if (dis_right != 1)
	  					// turn to the right
	    				m_tron->getPlayer(playerNr)->setDirection(sides[1]);
	  				else
	    				// turn to the left
	    				m_tron->getPlayer(playerNr)->setDirection(sides[0]);
          }
    }
}

/** find shortest path to all reachable fields */
void Intelligence::dijkstra(std::vector<std::vector <int>>& distances,int posX,int posY)
{
	
	int height = m_tron->getPlayField()->getHeight();
	int width = m_tron->getPlayField()->getWidth();
	
	//matrix of traversed fields
	std::vector<std::vector <int>> traversed(width, std::vector<int>(height));
	
	//Initialization of distances and traverse matrix
	for(int i = 0;i < width;i++)
	{
		for(int j = 0;j < height;j++)
		{
			distances[i][j] = 10000;
			traversed[i][j] = 0;
		}
	}
	
	//Starting point initialization
	distances[posX][posY] = 0;
	traversed[posX][posY] = 1;

	//Adjacent fields to starting position are set to 1 distance if valid
	
	//Setting left adjacent field to 1
 	int posLeftX = posX - 1;
	int posLeftY = posY;
	if(posLeftX < m_tron->getPlayField()->getWidth() && posLeftX >= 0 && posLeftY < m_tron->getPlayField()->getHeight() && posLeftY >= 0 && m_tron->getPlayField()->getObjectAt(posLeftX, posLeftY)->getObjectType() == ObjectType::Object)
	{
		distances[posLeftX][posLeftY] = 1;
	}
	
	//Setting right adjacent field to 1
	int posRightX = posX + 1;
	int posRightY = posY;
	if(posRightX < m_tron->getPlayField()->getWidth() && posRightX >= 0 && posRightY < m_tron->getPlayField()->getHeight() && posRightY >= 0 && m_tron->getPlayField()->getObjectAt(posRightX, posRightY)->getObjectType() == ObjectType::Object)
	{
		distances[posRightX][posRightY] = 1;
	}
	
	//Setting up adjacent field to 1
	int posUpX = posX ;
	int posUpY = posY + 1;
	if(posUpX < m_tron->getPlayField()->getWidth() && posUpX >= 0 &&posUpY < m_tron->getPlayField()->getHeight() && posUpY >= 0 && m_tron->getPlayField()->getObjectAt(posUpX,posUpY)->getObjectType() == ObjectType::Object)
	{
		distances[posUpX][posUpY] = 1;
	}
	
	//Setting downd adjacent field to 1
	int posDownX = posX;
	int posDownY = posY - 1;;
	if(posDownX < m_tron->getPlayField()->getWidth() && posDownX >= 0 && posDownY < m_tron->getPlayField()->getHeight() && posDownY >= 0 && m_tron->getPlayField()->getObjectAt(posDownX, posDownY)->getObjectType() == ObjectType::Object)
	{
		distances[posDownX][posDownY] = 1;
	}
	
	//These are coordinates for vertice n from dijkstra algorithm
	int nX;
	int nY;
	
	//Indicator for change in nX,nY 
	//If no change algorithm is finished
	int ind;
	
	//Infinite loop that breaks when no change in nX,nY
	while(1)
	{
	
		nX = 0;
		nY = 0;
		ind = -1;
		
		//Double for loop for finding minimum candiate for nX,nY
		for(int i = 0;i < width;i++)
		{
			for(int j = 0;j < height;j++)
			{
				if(distances[i][j] < distances[nX][nY] && traversed[i][j] != 1 && m_tron->getPlayField()->getObjectAt(nX, nY)->getObjectType() == ObjectType::Object   && distances[i][j] != 10000  )
				{
					nX = i;
					nY = j;
					ind = 1;
				}
			}	
		}
		
		//Marking the field as visited
		traversed[nX][nY] = 1;
		
		//If no change to nX,nY break
		if(ind == -1)
		{
			break;
		}

		//Checking adjacent fields and setting their distances to smallest so far	
		posLeftX = nX - 1;
		posLeftY = nY;
		
		//Checking adjacent fields to the left
		if(posLeftX < m_tron->getPlayField()->getWidth() && posLeftX >= 0 && posLeftY < m_tron->getPlayField()->getHeight() && posLeftY >= 0 && m_tron->getPlayField()->getObjectAt(posLeftX, posLeftY)->getObjectType() == ObjectType::Object)
		{
			if(distances[posLeftX][posLeftY] > (distances[nX][nY] + 1))
			{				
				distances[posLeftX][posLeftY] = distances[nX][nY] + 1;
			}
		}
		
		//Checking adjacent fields to the right
		posRightX = nX + 1;
		posRightY = nY;
		if(posRightX < m_tron->getPlayField()->getWidth() && posRightX >= 0 && posRightY < m_tron->getPlayField()->getHeight() && posRightY >= 0 && m_tron->getPlayField()->getObjectAt(posRightX, posRightY)->getObjectType() == ObjectType::Object)
		{
			if(distances[posRightX][posRightY] > (distances[nX][nY] + 1))
			{
				distances[posRightX][posRightY] = distances[nX][nY] + 1;
			}
		}
		
		//Checking adjacent fields to the up
		posUpX = nX ;
		posUpY = nY + 1;
		if(posUpX < m_tron->getPlayField()->getWidth() && posUpX >= 0 &&posUpY < m_tron->getPlayField()->getHeight() &&posUpY >= 0 && m_tron->getPlayField()->getObjectAt(posUpX,posUpY)->getObjectType() == ObjectType::Object)
		{
			if(distances[posUpX][posUpY] > (distances[nX][nY] + 1))
			{
				distances[posUpX][posUpY] = distances[nX][nY] + 1;
			}
		}
		
		//Checking adjacent fields to the down
		posDownX = nX;
		posDownY = nY - 1;
		if(posDownX < m_tron->getPlayField()->getWidth() && posDownX >= 0 && posDownY < m_tron->getPlayField()->getHeight() && posDownY >= 0 && m_tron->getPlayField()->getObjectAt(posDownX, posDownY)->getObjectType() == ObjectType::Object)
		{
			if(distances[posDownX][posDownY] > (distances[nX][nY] + 1))
			{
				distances[posDownX][posDownY] = distances[nX][nY] + 1;
			}		
		}
	}	
}

// This part is partly ported from
// xtron-1.1 by Rhett D. Jacobs <rhett@hotel.canberra.edu.au>
void Intelligence::think(int chance,int playerNr)
{
	if (opponentSkill() != 1 && opponentSkill() != 3  && opponentSkill() != 5)
	{
		int opponent=(playerNr==1)? 0 : 1;

		// determines left and right side
		PlayerDirections::Direction sides[2];
		sides[0] = PlayerDirections::None;
		sides[1] = PlayerDirections::None;
		// increments for moving to the different sides
		int flags[6]={0,0,0,0,0,0};
		int index[2];
		// distances to barrier
		int dis_forward,  dis_left, dis_right;

		dis_forward = dis_left = dis_right = 1;

		switch (m_tron->getPlayer(playerNr)->getDirection())
		{
			case PlayerDirections::Left:
				//forward flags
				flags[0] = -1;
				flags[1] = 0;

				//left flags
				flags[2] = 0;
				flags[3] = 1;

				// right flags
				flags[4] = 0;
				flags[5] = -1;

				//turns to either side
				sides[0] = PlayerDirections::Down;
				sides[1] = PlayerDirections::Up;
				break;
			case PlayerDirections::Right:
				flags[0] = 1;
				flags[1] = 0;
				flags[2] = 0;
				flags[3] = -1;
				flags[4] = 0;
				flags[5] = 1;
				sides[0] = PlayerDirections::Up;
				sides[1] = PlayerDirections::Down;
				break;
			case PlayerDirections::Up:
				flags[0] = 0;
				flags[1] = -1;
				flags[2] = -1;
				flags[3] = 0;
				flags[4] = 1;
				flags[5] = 0;
				sides[0] = PlayerDirections::Left;
				sides[1] = PlayerDirections::Right;
				break;
			case PlayerDirections::Down:
				flags[0] = 0;
				flags[1] = 1;
				flags[2] = 1;
				flags[3] = 0;
				flags[4] = -1;
				flags[5] = 0;
				sides[0] = PlayerDirections::Right;
				sides[1] = PlayerDirections::Left;
				break;
			default:
				break;
		}

		// check forward
		index[0] = m_tron->getPlayer(playerNr)->getX()+flags[0];
		index[1] = m_tron->getPlayer(playerNr)->getY()+flags[1];
		while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] < m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object)
		{
			dis_forward++;
			index[0] += flags[0];
			index[1] += flags[1];
		}

		// check left
		index[0] = m_tron->getPlayer(playerNr)->getX()+flags[2];
		index[1] = m_tron->getPlayer(playerNr)->getY()+flags[3];
		while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] < m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object)
		{
			dis_left++;
			index[0] += flags[2];
			index[1] += flags[3];
		}

		// check right
		index[0] = m_tron->getPlayer(playerNr)->getX()+flags[4];
		index[1] = m_tron->getPlayer(playerNr)->getY()+flags[5];
		while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] <  m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object)
		{
			dis_right++;
			index[0] += flags[4];
			index[1] += flags[5];
		}

		// distances to opponent
		int hor_dis=0; // negative is opponent to the right
		int vert_dis=0; // negative is opponent to the bottom
		hor_dis = m_tron->getPlayer(playerNr)->getX() - m_tron->getPlayer(opponent)->getX();
		vert_dis = m_tron->getPlayer(playerNr)->getY() - m_tron->getPlayer(opponent)->getY();

		int opForwardDis=0; // negative is to the back
		int opSideDis=0;  // negative is to the left
		bool opMovesOppositeDir=false;
		bool opMovesSameDir=false;
		bool opMovesRight=false;
		bool opMovesLeft=false;

		switch (m_tron->getPlayer(playerNr)->getDirection())
		{
			case PlayerDirections::Up:
				opForwardDis=vert_dis;
				opSideDis=-hor_dis;
				if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Down)
					opMovesOppositeDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Up)
					opMovesSameDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Left)
					opMovesLeft=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Right)
					opMovesRight=true;
				break;
			case PlayerDirections::Down:
				opForwardDis=-vert_dis;
				opSideDis=hor_dis;
				if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Up)
					opMovesOppositeDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Down)
					opMovesSameDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Left)
					opMovesRight=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Right)
					opMovesLeft=true;
				break;
			case PlayerDirections::Left:
				opForwardDis=hor_dis;
				opSideDis=vert_dis;
				if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Right)
					opMovesOppositeDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Left)
					opMovesSameDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Down)
					opMovesLeft=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Up)
					opMovesRight=true;
				break;
			case PlayerDirections::Right:
				opForwardDis=-hor_dis;
				opSideDis=-vert_dis;
				if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Left)
					opMovesOppositeDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Right)
					opMovesSameDir=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Up)
					opMovesLeft=true;
				else if(m_tron->getPlayer(opponent)->getDirection()==PlayerDirections::Down)
					opMovesRight=true;
				break;
			default:
				break;
		}
		int doPercentage = 100;
		switch(opponentSkill())
		{
			case 1:
				// Never reached
				break;
			case 2:
				doPercentage=5;
				break;
			case 4:
				doPercentage=100;
				break;
		}

		// if opponent moves the opposite direction as we
		if(opMovesOppositeDir)
		{
			// if opponent is in front
			if(opForwardDis>0)
			{
				// opponent is to the right and we have the chance to block the way
				if(opSideDis>0 && opSideDis < opForwardDis && opSideDis < dis_right && opForwardDis < m_lookForward)
				{
					if ((int)m_random.getLong(100) <= doPercentage || dis_forward==1)
						m_tron->getPlayer(playerNr)->setDirection(sides[1]); // turn right
				}
				// opponent is to the left and we have the chance to block the way
				else if(opSideDis<0 && -opSideDis < opForwardDis && -opSideDis < dis_left && opForwardDis < m_lookForward)
				{
					if ((int)m_random.getLong(100) <= doPercentage || dis_forward==1)
						m_tron->getPlayer(playerNr)->setDirection(sides[0]); // turn left
				}
				// if we can do nothing, go forward
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

					if(!(dis_left == 1 && dis_right == 1))
						if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
							changeDirection(playerNr,dis_right,dis_left);
				}
			}
			// opponent is in back of us and moves away: do nothing
			else if(dis_forward < m_lookForward)
			{
				dis_forward = 100 - 100/dis_forward;

				if(!(dis_left == 1 && dis_right == 1))
					if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
							changeDirection(playerNr,dis_right,dis_left);
			}
		} // end  if(opMovesOppositeDir)
		else if(opMovesSameDir)
		{
			// if opponent is to the back
			if(opForwardDis < 0)
			{
					// opponent is to the right and we have the chance to block the way
				if(opSideDis>0 && opSideDis < -opForwardDis && opSideDis < dis_right)
				{
					if ((int)m_random.getLong(100) <= doPercentage || dis_forward==1)
						m_tron->getPlayer(playerNr)->setDirection(sides[1]); // turn right
				}
				// opponent is to the left and we have the chance to block the way
				else if(opSideDis<0 && -opSideDis < -opForwardDis && -opSideDis < dis_left)
				{
					if ((int)m_random.getLong(100) <= doPercentage || dis_forward==1)
						m_tron->getPlayer(playerNr)->setDirection(sides[0]); // turn left
				}
				// if we can do nothing, go forward
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

						if(!(dis_left == 1 && dis_right == 1))
							if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
								changeDirection(playerNr,dis_right,dis_left);
				}
			}
			// opponent is in front of us and moves away
			else if(dis_forward < m_lookForward)
			{
				dis_forward = 100 - 100/dis_forward;

					if(!(dis_left == 1 && dis_right == 1))
						if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
							changeDirection(playerNr,dis_right,dis_left);
			}
		} // end if(opMovesSameDir)
		else if(opMovesRight)
		{
			// opponent is in front of us
			if(opForwardDis>0)
			{
				// opponent is to the left
				if(opSideDis < 0 && -opSideDis < opForwardDis && -opSideDis < dis_left)
				{
					if(opForwardDis < m_lookForward && dis_left > m_lookForward)
					{
						if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
							changeDirection(playerNr,dis_right,dis_left);
					}
					else if(dis_forward < m_lookForward)
					{
						dis_forward = 100 - 100/dis_forward;

							if(!(dis_left == 1 && dis_right == 1))
								if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
									changeDirection(playerNr,dis_right,dis_left);
					}
				}
				// op is to the right and moves away, but maybe we can block him
				else if(opSideDis>=0 && opSideDis < dis_right)
				{
					if(opForwardDis < m_lookForward && dis_right > m_lookForward)
					{
						if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
							m_tron->getPlayer(playerNr)->setDirection(sides[1]); // turn right
					}
					else if(dis_forward < m_lookForward)
					{
						dis_forward = 100 - 100/dis_forward;

							if(!(dis_left == 1 && dis_right == 1))
								if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
									changeDirection(playerNr,dis_right,dis_left);
					}
				}
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

						if(!(dis_left == 1 && dis_right == 1))
							if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
								changeDirection(playerNr,dis_right,dis_left);
				}
			}
			// opponent is in the back of us
			else
			{
				// opponent is right from us and we already blocked him
				if(opSideDis>0 && opForwardDis < m_lookForward && opSideDis < dis_right)
				{
					if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
						changeDirection(playerNr,dis_right,dis_left);
				}
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

						if(!(dis_left == 1 && dis_right == 1))
							if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
								changeDirection(playerNr,dis_right,dis_left);
				}
			}
		} // end if(opMovesRight)
		else if(opMovesLeft)
		{
			// opponent is in front of us
			if(opForwardDis>0)
			{
				// opponent is to the right, moves towards us and could block us
				if(opSideDis > 0 && opSideDis < opForwardDis && opSideDis < dis_right)
				{
					if(opForwardDis < m_lookForward && dis_right > m_lookForward)
					{
						if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
							changeDirection(playerNr,dis_right,dis_left);
					}
					else if(dis_forward < m_lookForward)
					{
						dis_forward = 100 - 100/dis_forward;

						if(!(dis_left == 1 && dis_right == 1))
							if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
								changeDirection(playerNr,dis_right,dis_left);
					}
				}
				// op is to the left and moves away, but maybe we can block him
				else if(opSideDis<=0 && opSideDis < dis_left)
				{
					if(opForwardDis < m_lookForward && dis_left > m_lookForward)
					{
						if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
							m_tron->getPlayer(playerNr)->setDirection(sides[0]); // m_turn left
						}
					else if(dis_forward < m_lookForward)
					{
						dis_forward = 100 - 100/dis_forward;

						if(!(dis_left == 1 && dis_right == 1))
							if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
								changeDirection(playerNr,dis_right,dis_left);
					}

				}
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

					if(!(dis_left == 1 && dis_right == 1))
						if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
							changeDirection(playerNr,dis_right,dis_left);
				}
			}
			// opponent is in the back of us
			else //if(opForwardDis<=0)
			{
				// opponent is left from us and we already blocked him
				if(opSideDis<0 && opForwardDis < m_lookForward && -opSideDis < dis_left)
				{
					if ((int)m_random.getLong(100) <= doPercentage/2 || dis_forward==1)
						changeDirection(playerNr,dis_right,dis_left);
				}
				else if(dis_forward < m_lookForward)
				{
					dis_forward = 100 - 100/dis_forward;

					if(!(dis_left == 1 && dis_right == 1))
						if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 1)
							changeDirection(playerNr,dis_right,dis_left);
				}
			}
		} // end if(opMovesLeft)

	}
	// This part is completely ported from
	// xtron-1.1 by Rhett D. Jacobs <rhett@hotel.canberra.edu.au>
	else if (opponentSkill() == 1) // Settings::skill() == Settings::EnumSkill::Easy
	{
		PlayerDirections::Direction sides[2];
		sides[0] = PlayerDirections::None;
		sides[1] = PlayerDirections::None;
		int flags[6] = {0,0,0,0,0,0};
		int index[2];
		int dis_forward,  dis_left, dis_right;

		dis_forward = dis_left = dis_right = 1;

		switch (m_tron->getPlayer(playerNr)->getDirection()) {
			case PlayerDirections::Left:
				//forward flags
				flags[0] = -1;
				flags[1] = 0;
				//left flags
				flags[2] = 0;
				flags[3] = 1;
				// right flags
				flags[4] = 0;
				flags[5] = -1;
				//turns to either side
				sides[0] = PlayerDirections::Down;
				sides[1] = PlayerDirections::Up;
				break;
			case PlayerDirections::Right:
				flags[0] = 1;
				flags[1] = 0;
				flags[2] = 0;
				flags[3] = -1;
				flags[4] = 0;
				flags[5] = 1;
				sides[0] = PlayerDirections::Up;
				sides[1] = PlayerDirections::Down;
				break;
			case PlayerDirections::Up:
				flags[0] = 0;
				flags[1] = -1;
				flags[2] = -1;
				flags[3] = 0;
				flags[4] = 1;
				flags[5] = 0;
				sides[0] = PlayerDirections::Left;
				sides[1] = PlayerDirections::Right;
				break;
			case PlayerDirections::Down:
				flags[0] = 0;
				flags[1] = 1;
				flags[2] = 1;
				flags[3] = 0;
				flags[4] = -1;
				flags[5] = 0;
				sides[0] = PlayerDirections::Right;
				sides[1] = PlayerDirections::Left;
				break;
			default:
				break;
		}

		// check forward
		index[0] = m_tron->getPlayer(playerNr)->getX() + flags[0];
		index[1] = m_tron->getPlayer(playerNr)->getY() + flags[1];
		while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] < m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object) {
			dis_forward++;
			index[0] += flags[0];
			index[1] += flags[1];
		}

		if (dis_forward < m_lookForward)
		{
			dis_forward = 100 - 100 / dis_forward;

			// check left
			index[0] = m_tron->getPlayer(playerNr)->getX() + flags[2];
			index[1] = m_tron->getPlayer(playerNr)->getY() + flags[3];
			while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] < m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object) {
				dis_left++;
				index[0] += flags[2];
				index[1] += flags[3];
			}

			// check right
			index[0] = m_tron->getPlayer(playerNr)->getX() + flags[4];
			index[1] = m_tron->getPlayer(playerNr)->getY() + flags[5];
			while (index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] <  m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object) {
				dis_right++;
				index[0] += flags[4];
				index[1] += flags[5];
			}
			if(!(dis_left == 1 && dis_right == 1)) {
				if ((int)m_random.getLong(100) >= dis_forward || dis_forward == 0) {
					// change direction
					if ((int)m_random.getLong(100) <= (100*dis_left)/(dis_left+dis_right)) {
						if (dis_left != 1)
							// turn to the left
							m_tron->getPlayer(playerNr)->setDirection(sides[0]);
						else
							// turn to the right
							m_tron->getPlayer(playerNr)->setDirection(sides[1]);
					}
					else {
						if (dis_right != 1)
							// turn to the right
							m_tron->getPlayer(playerNr)->setDirection(sides[1]);
						else
							// turn to the left
							m_tron->getPlayer(playerNr)->setDirection(sides[0]);
					}
				}
			}
		}
	//Very Hard difficulty
	}else if (opponentSkill() == 5)
	{
		//100 percent chance that will play a good move
		Intelligence::think_better(100,playerNr);

	//Medium - adapting difficulty
	}else if (opponentSkill() == 3)
	{
		Intelligence::think_better(chance,playerNr);
		
	}
}

//Better helper AI function based on dijksta path finding heuristics
void Intelligence::think_better(int chance,int playerNr)
{
	
	int opponent=(playerNr==1)? 0 : 1;
	
	//First we define sides and flags depending on orientation of the palyer
	//We will use side later to turn left or right
	PlayerDirections::Direction sides[2];
	sides[0] = PlayerDirections::None;
	sides[1] = PlayerDirections::None;
	int flags[6] = {0,0,0,0,0,0};
	
	int index[2];
	switch (m_tron->getPlayer(playerNr)->getDirection())
	{
		case PlayerDirections::Left:
			//forward flags
			flags[0] = -1;
			flags[1] = 0;
			//left flags
			flags[2] = 0;
			flags[3] = 1;
			// right flags
			flags[4] = 0;
			flags[5] = -1;
			//turns to either side
			sides[0] = PlayerDirections::Down;
			sides[1] = PlayerDirections::Up;
			break;
		case PlayerDirections::Right:
			flags[0] = 1;
			flags[1] = 0;
			flags[2] = 0;
			flags[3] = -1;
			flags[4] = 0;
			flags[5] = 1;
			sides[0] = PlayerDirections::Up;
			sides[1] = PlayerDirections::Down;
			break;
		case PlayerDirections::Up:
			flags[0] = 0;
			flags[1] = -1;
			flags[2] = -1;
			flags[3] = 0;
			flags[4] = 1;
			flags[5] = 0;
			sides[0] = PlayerDirections::Left;
			sides[1] = PlayerDirections::Right;
			break;
		case PlayerDirections::Down:
			flags[0] = 0;
			flags[1] = 1;
			flags[2] = 1;
			flags[3] = 0;
			flags[4] = -1;
			flags[5] = 0;
			sides[0] = PlayerDirections::Right;
			sides[1] = PlayerDirections::Left;
			break;
		default:
			break;
	}
	
	int width = m_tron->getPlayField()->getWidth();
	int height = m_tron->getPlayField()->getHeight();
	
	//Get position of the AI bot
	int aiX = m_tron->getPlayer(playerNr)->getX();
	int aiY = m_tron->getPlayer(playerNr)->getY();
	
	//Get position of the opponent
	int oppX = m_tron->getPlayer(opponent)->getX();
	int oppY = m_tron->getPlayer(opponent)->getY();
	
	//Define distances matrix for opponent
	std::vector<std::vector <int>> distancesOpp(width, std::vector<int>(height));
	
	//Define distances matrix for AI 
	std::vector<std::vector <int>> distancesAI(width, std::vector<int>(height));
	
	//Fill opponent distances matrix with shortest paths using Dijstra algorithm
	Intelligence::dijkstra(distancesOpp,oppX,oppY);
	
	//move will determine which move we will play at the end
	int move = 0;
	
	//Counters which count for how many field on board AI has better position then opponent
	int maxBestPositionCounter = 0;
	int BestPositionCounter = 0;
	
	/* Loop that evaluates all 3 possible moves and determines which is the best
	by comparing how many filelds AI can reach first vs how many can opponent
	the more is better and we choose that move,
	this is main heuristic for this algorithm */
	
	for(int i = 0;i < 3;i++)
	{
		index[0] = aiX + flags[2*i];
		index[1] = aiY + flags[2*i + 1];
		if(index[0] < m_tron->getPlayField()->getWidth() && index[0] >= 0 && index[1] < m_tron->getPlayField()->getHeight() && index[1] >= 0 && m_tron->getPlayField()->getObjectAt(index[0], index[1])->getObjectType() == ObjectType::Object)
		{
			Intelligence::dijkstra(distancesAI,index[0],index[1]);
			BestPositionCounter = 0;
			for(int i = 0;i < width;i++)
			{
				for(int j = 0;j < height;j++)
				{
					if(distancesAI[i][j] < distancesOpp[i][j] - 1)
					{
						BestPositionCounter ++;
					}
				}
			}
			

			if(BestPositionCounter > maxBestPositionCounter)
			{
				maxBestPositionCounter = BestPositionCounter;
				move = i;	
			}
		}
	} 
	
	//This is chance of algorithm making a mistake
	int error = (int)m_random.getLong(100);
	if(error > chance)
	{
		move = 0;
	}
	//Make a move, turn left
	if(move == 1)
	{
		m_tron->getPlayer(playerNr)->setDirection(sides[0]);
		
	//Make a move, turn right
	}else if(move == 2)
	{
		m_tron->getPlayer(playerNr)->setDirection(sides[1]);
	}

}


