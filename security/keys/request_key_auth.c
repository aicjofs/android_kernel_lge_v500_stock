/* Request key authorisation token key definition.
 *
 * Copyright (C) 2005 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * See Documentation/security/keys-request-key.txt
 */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "internal.h"

static int request_key_auth_instantiate(struct key *, const void *, size_t);
static void request_key_auth_describe(const struct key *, struct seq_file *);
static void request_key_auth_revoke(struct key *);
static void request_key_auth_destroy(struct key *);
static long request_key_auth_read(const struct key *, char __user *, size_t);

/*
                                                     
 */
struct key_type key_type_request_key_auth = {
	.name		= ".request_key_auth",
	.def_datalen	= sizeof(struct request_key_auth),
	.instantiate	= request_key_auth_instantiate,
	.describe	= request_key_auth_describe,
	.revoke		= request_key_auth_revoke,
	.destroy	= request_key_auth_destroy,
	.read		= request_key_auth_read,
};

/*
                                               
 */
static int request_key_auth_instantiate(struct key *key,
					const void *data,
					size_t datalen)
{
	key->payload.data = (struct request_key_auth *) data;
	return 0;
}

/*
                                   
 */
static void request_key_auth_describe(const struct key *key,
				      struct seq_file *m)
{
	struct request_key_auth *rka = key->payload.data;

	seq_puts(m, "key:");
	seq_puts(m, key->description);
	if (key_is_instantiated(key))
		seq_printf(m, " pid:%d ci:%zu", rka->pid, rka->callout_len);
}

/*
                                                                  
                                       
 */
static long request_key_auth_read(const struct key *key,
				  char __user *buffer, size_t buflen)
{
	struct request_key_auth *rka = key->payload.data;
	size_t datalen;
	long ret;

	datalen = rka->callout_len;
	ret = datalen;

	/*                              */
	if (buffer && buflen > 0) {
		if (buflen > datalen)
			buflen = datalen;

		if (copy_to_user(buffer, rka->callout_info, buflen) != 0)
			ret = -EFAULT;
	}

	return ret;
}

/*
                                                   
  
                                        
 */
static void request_key_auth_revoke(struct key *key)
{
	struct request_key_auth *rka = key->payload.data;

	kenter("{%d}", key->serial);

	if (rka->cred) {
		put_cred(rka->cred);
		rka->cred = NULL;
	}
}

/*
                                                    
 */
static void request_key_auth_destroy(struct key *key)
{
	struct request_key_auth *rka = key->payload.data;

	kenter("{%d}", key->serial);

	if (rka->cred) {
		put_cred(rka->cred);
		rka->cred = NULL;
	}

	key_put(rka->target_key);
	key_put(rka->dest_keyring);
	kfree(rka->callout_info);
	kfree(rka);
}

/*
                                                                         
                                        
 */
struct key *request_key_auth_new(struct key *target, const void *callout_info,
				 size_t callout_len, struct key *dest_keyring)
{
	struct request_key_auth *rka, *irka;
	const struct cred *cred = current->cred;
	struct key *authkey = NULL;
	char desc[20];
	int ret;

	kenter("%d,", target->serial);

	/*                        */
	rka = kmalloc(sizeof(*rka), GFP_KERNEL);
	if (!rka) {
		kleave(" = -ENOMEM");
		return ERR_PTR(-ENOMEM);
	}
	rka->callout_info = kmalloc(callout_len, GFP_KERNEL);
	if (!rka->callout_info) {
		kleave(" = -ENOMEM");
		kfree(rka);
		return ERR_PTR(-ENOMEM);
	}

	/*                                                                   
                    */
	if (cred->request_key_auth) {
		/*                                                 */
		down_read(&cred->request_key_auth->sem);

		/*                                                     
                                       */
		if (test_bit(KEY_FLAG_REVOKED, &cred->request_key_auth->flags))
			goto auth_key_revoked;

		irka = cred->request_key_auth->payload.data;
		rka->cred = get_cred(irka->cred);
		rka->pid = irka->pid;

		up_read(&cred->request_key_auth->sem);
	}
	else {
		/*                                            */
		rka->cred = get_cred(cred);
		rka->pid = current->pid;
	}

	rka->target_key = key_get(target);
	rka->dest_keyring = key_get(dest_keyring);
	memcpy(rka->callout_info, callout_info, callout_len);
	rka->callout_len = callout_len;

	/*                       */
	sprintf(desc, "%x", target->serial);

	authkey = key_alloc(&key_type_request_key_auth, desc,
			    cred->fsuid, cred->fsgid, cred,
			    KEY_POS_VIEW | KEY_POS_READ | KEY_POS_SEARCH |
			    KEY_USR_VIEW, KEY_ALLOC_NOT_IN_QUOTA);
	if (IS_ERR(authkey)) {
		ret = PTR_ERR(authkey);
		goto error_alloc;
	}

	/*                        */
	ret = key_instantiate_and_link(authkey, rka, 0, NULL, NULL);
	if (ret < 0)
		goto error_inst;

	kleave(" = {%d,%d}", authkey->serial, atomic_read(&authkey->usage));
	return authkey;

auth_key_revoked:
	up_read(&cred->request_key_auth->sem);
	kfree(rka->callout_info);
	kfree(rka);
	kleave("= -EKEYREVOKED");
	return ERR_PTR(-EKEYREVOKED);

error_inst:
	key_revoke(authkey);
	key_put(authkey);
error_alloc:
	key_put(rka->target_key);
	key_put(rka->dest_keyring);
	kfree(rka->callout_info);
	kfree(rka);
	kleave("= %d", ret);
	return ERR_PTR(ret);
}

/*
                                                                   
 */
static int key_get_instantiation_authkey_match(const struct key *key,
					       const void *_id)
{
	struct request_key_auth *rka = key->payload.data;
	key_serial_t id = (key_serial_t)(unsigned long) _id;

	return rka->target_key->serial == id;
}

/*
                                                                      
                          
 */
struct key *key_get_instantiation_authkey(key_serial_t target_id)
{
	const struct cred *cred = current_cred();
	struct key *authkey;
	key_ref_t authkey_ref;

	authkey_ref = search_process_keyrings(
		&key_type_request_key_auth,
		(void *) (unsigned long) target_id,
		key_get_instantiation_authkey_match,
		cred);

	if (IS_ERR(authkey_ref)) {
		authkey = ERR_CAST(authkey_ref);
		if (authkey == ERR_PTR(-EAGAIN))
			authkey = ERR_PTR(-ENOKEY);
		goto error;
	}

	authkey = key_ref_to_ptr(authkey_ref);
	if (test_bit(KEY_FLAG_REVOKED, &authkey->flags)) {
		key_put(authkey);
		authkey = ERR_PTR(-EKEYREVOKED);
	}

error:
	return authkey;
}
