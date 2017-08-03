#include <libssh/libssh.h> 
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct Client{
	string name;
	string hote;
	string user;
	ssh_session session;
	ssh_channel channel;
}; 

static vector<Client> liste_clients;


int read_file(const char* file_name) {
	ifstream file(file_name, ios::in);
	int host = 0;
	
	if (file) {
		string line;
		while (getline(file, line)) {
			if (host == 1 && line != "") {
				int space = line.find(' ');
				Client newClient;
				newClient.user = line.substr(0, space);
				newClient.hote = line.substr(space + 1, line.length());
				cout << "host : " << newClient.hote << endl;
				int point = newClient.hote.find('.');
				newClient.name = newClient.hote.substr(0, point);
				
				liste_clients.push_back(newClient);
			}
			if (line == "#HOSTS")
				host = 1;
			if (line == "" && host == 1)
				host = 0;
		}
		
		file.close();
	}
	else {
		cerr << "Ouverture fichier impossible." << endl;
		return -1;
	}
	
	return 0;
}


/*
 * Authentification par clé ne fonctionne pas encore
 * Obligé de passer par mot de passe
 * Correction à venir dans une prochaine version
 * (Pas le plus urgent pour le moment, toutefois)
 * 
 */

int verify_knownhost(ssh_session session)
{
	int state, hlen;
	unsigned char *hash = NULL;
	char *hexa;
	char buf[10];
	state = ssh_is_server_known(session);
	hlen = ssh_get_pubkey_hash(session, &hash);
	if (hlen < 0)
		return -1;
	switch (state)
	{
		case SSH_SERVER_KNOWN_OK:
			break; /* ok */
			
		case SSH_SERVER_KNOWN_CHANGED:
			fprintf(stderr, "Host key for server changed: it is now:\n");
			ssh_print_hexa("Public key hash", hash, hlen);
			fprintf(stderr, "For security reasons, connection will be stopped\n");
			free(hash);
			return -1;
			
		case SSH_SERVER_FOUND_OTHER:
			fprintf(stderr, "The host key for this server was not found but an other"
			"type of key exists.\n");
			fprintf(stderr, "An attacker might change the default server key to"
			"confuse your client into thinking the key does not exist\n");
			free(hash);
			return -1;
			
		case SSH_SERVER_FILE_NOT_FOUND:
			fprintf(stderr, "Could not find known host file.\n");
			fprintf(stderr, "If you accept the host key here, the file will be"
			"automatically created.\n");
			
      /* fallback to SSH_SERVER_NOT_KNOWN behavior */
		case SSH_SERVER_NOT_KNOWN:
			hexa = ssh_get_hexa(hash, hlen);
			fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
			fprintf(stderr, "Public key hash: %s\n", hexa);
			free(hexa);
			if (fgets(buf, sizeof(buf), stdin) == NULL)
			{
				free(hash);
				return -1;
			}
			if (strncasecmp(buf, "yes", 3) != 0)
			{
				free(hash);
				return -1;
			}
			if (ssh_write_knownhost(session) < 0)
			{
				fprintf(stderr, "Error %s\n", strerror(errno));
				free(hash);
				return -1;
			}
			break;
			
		case SSH_SERVER_ERROR:
			fprintf(stderr, "Error %s", ssh_get_error(session));
			free(hash);
			return -1;
	}
	free(hash);
	return 0;
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int send_cmd(Client client, string cmd) {
	char buffer[2048];
	int nbytes = 0;
	
	int nwritten;
	int rw = 1; 
	
	//while (ssh_channel_is_open(client.channel) && !ssh_channel_is_eof(client.channel)) {
		
		//lecture du terminal distant
		
		//lecture commande
		//nbytes = read(0, buffer, sizeof(buffer));
		
		nbytes = cmd.length();
		nwritten = ssh_channel_write(client.channel, cmd.c_str(), nbytes);
		if (nwritten != nbytes) 
			return SSH_ERROR;
	
		//lecture du terminal distant
		nbytes = ssh_channel_read(client.channel, buffer, sizeof(buffer), 0);
		if (nbytes < 0) 
			return SSH_ERROR;
		//if (nbytes > 0)	{
			//nwritten = write(1, buffer, nbytes);
			//if (nwritten != nbytes) 
				//return SSH_ERROR;
		//}
		//cout << "read : " << nbytes << endl;
		nbytes = ssh_channel_read(client.channel, buffer, sizeof(buffer), 0);
		if (nbytes < 0) 
			return SSH_ERROR;
			
		//if (nbytes > 0)	{
			//nwritten = write(1, buffer, nbytes);
			//if (nwritten != nbytes) 
				//return SSH_ERROR;
		//}

		//cout << "read : " << nbytes << endl;
		
		nbytes = ssh_channel_read(client.channel, buffer, sizeof(buffer), 0);
		if (nbytes < 0) 
			return SSH_ERROR;
		if (nbytes > 0)	{
			nwritten = write(1, buffer, nbytes);
			if (nwritten != nbytes) 
				return SSH_ERROR;
		}

		cout << "read : " << nbytes << endl;
	//}
	
	return SSH_OK;
}

int create_ssh_session() {
	int verbosity = SSH_LOG_NOLOG;
	int port = 22;
	int rc;
	
	for(int i = 0; i < liste_clients.size(); i++) {
		cout << "Connection to " << liste_clients[i].hote << endl;
		liste_clients[i].session = ssh_new();
		
		if (liste_clients[i].session == NULL)
			return -1;
			
		ssh_options_set(liste_clients[i].session, SSH_OPTIONS_HOST, (liste_clients[i].hote).c_str());
		ssh_options_set(liste_clients[i].session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
		ssh_options_set(liste_clients[i].session, SSH_OPTIONS_PORT, &port);
		ssh_options_set(liste_clients[i].session, SSH_OPTIONS_USER, (liste_clients[i].user).c_str());
		
		rc = ssh_connect(liste_clients[i].session);
	
		if (rc != SSH_OK) {
			fprintf(stderr, "Error connecting to server: %s\n", ssh_get_error(liste_clients[i].session));
			exit(-1);
		}
		
		if ( verify_knownhost(liste_clients[i].session) < 0 ) {
			ssh_disconnect(liste_clients[i].session);
			ssh_free(liste_clients[i].session);
			cout << "failed to connect" << endl;
			return -1;
		}
		
		/*
		rc = ssh_userauth_publickey_auto(liste_clients[i].session, NULL, getpass("Password: "));
		if (rc == SSH_AUTH_ERROR)
		{
			fprintf(stderr, "Authentication failed: %s\n",
			ssh_get_error(liste_clients[i].session));
			return SSH_AUTH_ERROR;
		}
		else
			cout << "Now connected to " << liste_clients[i].hote << endl << endl;
			*/
		rc = ssh_userauth_publickey_auto(liste_clients[i].session, NULL, NULL);
		
		if (rc == SSH_AUTH_ERROR) {
			fprintf(stderr, "Authentication failed: %s\n",
			ssh_get_error(liste_clients[i].session));
			return SSH_AUTH_ERROR;
		}
		else
			cout << "Now connected to " << liste_clients[i].hote << endl << endl;
	}
	
	return 0;
}

int create_channel() {
	
	int rc;
	int nbytes = 0;
	char buffer[2048];
	for(int i = 0; i < liste_clients.size(); i++) {	
		cout << "creating channel for " << liste_clients[i].hote << endl;
		liste_clients[i].channel = ssh_channel_new(liste_clients[i].session);
		if (liste_clients[i].channel == NULL)
			return SSH_ERROR;
		rc = ssh_channel_open_session(liste_clients[i].channel);
		
		if (rc != SSH_OK) {
			fprintf(stderr, "creation new ssh channel failed\n");
			ssh_channel_free(liste_clients[i].channel);
			return rc;
		}
		
		rc = ssh_channel_request_pty(liste_clients[i].channel);
		if (rc != SSH_OK) 
			return rc;
			rc = ssh_channel_change_pty_size(liste_clients[i].channel, 80, 24);
		if (rc != SSH_OK) 
				return rc;
		rc = ssh_channel_request_shell(liste_clients[i].channel);
		if (rc != SSH_OK)
			return rc;
		
		cout << "channel created" << endl;

		//Lecture du message de connexion que l'on ne veut pas afficher
		nbytes = ssh_channel_read(liste_clients[i].channel, buffer, sizeof(buffer), 0);
		if (nbytes < 0) 
			return SSH_ERROR;
	}
		
	return 0;
}

int switch_to_serv(Client client) {
	int rc;
	char buffer[2048];
	int nbytes;
	int nwritten;
	
	while (ssh_channel_is_open(client.channel) && !ssh_channel_is_eof(client.channel)) {
		
		//lecture du terminal distant
		nbytes = ssh_channel_read(client.channel, buffer, sizeof(buffer), 0);
		if (nbytes < 0) 
			return SSH_ERROR;
		if (nbytes > 0)	{
			nwritten = write(1, buffer, nbytes);
			if (nwritten != nbytes) 
				return SSH_ERROR;
		}

		//lecture commande
		nbytes = read(0, buffer, sizeof(buffer));
		
		if (strcmp(buffer, "exit") == 0)
			break;
			
		//ecriture sur terminal distant
		if (nbytes < 0) 
			return SSH_ERROR;
		if (nbytes > 0) {
			nwritten = ssh_channel_write(client.channel, buffer, nbytes);
			if (nwritten != nbytes) 
				return SSH_ERROR;
		}
	}
	
	return 0;
}

int send_file(Client client, string file){
	//first create archive
	string cmd = "gzip ";
	cmd.append(file);
	cout << "commande : " << cmd << endl;
	//system(cmd.c_str());
	
	//then send archive
	string newFile = file;
	newFile.append(".gz");
	cmd = "scp ";
	cmd.append(newFile);
	cmd.append(" ");
	cmd.append(client.user);
	cmd.append("@");
	cmd.append(client.hote);
	cmd.append(":~");
	cout << "finale cmd : " << cmd << endl;
	
	//system(cmd.c_str());
	return 0;
}

int exit_prog() {
	cout << "now disconnecting from every server" << endl;
	for(int i = 0; i < liste_clients.size(); i++) {
		ssh_channel_send_eof(liste_clients[i].channel);
		ssh_channel_close(liste_clients[i].channel);
		ssh_channel_free(liste_clients[i].channel);
		
		ssh_disconnect(liste_clients[i].session);
		ssh_free(liste_clients[i].session);
		
		cout << "disconnected from " << liste_clients[i].hote << endl;
	}
	
	return 0;
}


int main(int argc, char* argv[]) {
	

	read_file(argv[1]);
	create_ssh_session();
	create_channel();
	//send_file(liste_clients[0], "host.conf");
	//switch_to_serv(liste_clients[0]);
	
	int error = send_cmd(liste_clients[0], "ls\n");
	cout << "error: " << error << endl;
	exit_prog();
}
