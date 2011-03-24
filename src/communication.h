/**
 * \file communication.h
 * \brief Fichier de définition pour la communication
 * \author Stanislas BERTRAND
 * \version 0.1
 * \date 24 mars 2011
 *
 *	Fichier commun au développement pour le PDA et pour le robot
 *
 */


/**
 * \def COMBINE_CMD_MOD(CMD,MOD)
 * \brief Combine la commande et ses modes
 * \param CMD Commande
 * \param MOD Mode
 * \return (CMD | MOD)
 */
#define COMBINE_CMD_MOD(CMD,MOD) (CMD | MOD)

/**
 * \brief commande déplacement
 *
 */
#define CMD_DPL			0x20

/**
 * \brief acknowledge commande déplacement
 *
 */
#define CMD_DPL_ACK		0x30
/**
 * \brief commande environnement
 *
 */
#define CMD_ENV			0x40

/**
 * \brief acknowledge commande environement
 *
 */
#define CMD_ENV_ACK		0x50

/**
 * \brief mode environement capteur 1
 *
 */
#define MOD_ENV_CAP1		0x01
/**
 * \brief mode environement capteur 2
 *
 */
#define MOD_ENV_CAP2		0x02
/**
 * \brief mode environement capteur 3
 *
 */
#define MOD_ENV_CAP3		0x04
/**
 * \brief mode environement capteur 4
 *
 */
#define MOD_ENV_CAP4		0x08

/**
 * \brief mode deplacement axes.
 *
 * Définit l'interprétation des 2 octets de déplacement.
 *
 *	0 pour Moteur D,G
 *
 *	1 pour Axes X,Y
 */
#define MOD_DPL_AXES_XY		0x01
/**
 * \brief field of view for the environment
 *
 */
#define ENV_FOV			160
/**
 * \brief Half of the field of view for the environment
 *
 */
#define ENV_FOV_HALF		160
/**
 * \brief Incrément pour le parcours du FOV
 *
 */
#define ENV_ANGLE_INC		10


