# README Stage 2017

### Utilisation des différents fichiers
  - farmerD.cpp : ce fichier, à compiler via le Makefile, est le farmer D principal, c'est lui qui va se connecter aux différents serveurs esclaves pour y lancer les solvers et les contrôler. Il prend en argument le fichier "host.conf".
  - solver.py : c'est ce programme qui sera lancé sur les serveurs esclaves. Il va lancer autant de solver en arrière plan qu'il y a de coeur sur la machine. Ce programme fonctionne comme un intérpréteur, il peut donc directement recevoir des commandes spécifiques. (Détail des commandes à venir).
  - host.conf : C'est le fichier de configuration qui doit être donné en argument au fichier farmerD.cpp. Ce fichier contient les htes auxquels doit se connecter le farmer. Chaque ligne contient le nom d'utilisateur suivi, après un espace, du nom de l'hôte en entier auquel se connecter.
