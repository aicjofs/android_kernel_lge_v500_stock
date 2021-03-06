/*
 * In-kernel MOUNT protocol client
 *
 * Copyright (C) 1997, Olaf Kirch <okir@monad.swb.de>
 */

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/uio.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/sunrpc/clnt.h>
#include <linux/sunrpc/sched.h>
#include <linux/nfs_fs.h>
#include "internal.h"

#ifdef NFS_DEBUG
# define NFSDBG_FACILITY	NFSDBG_MOUNT
#endif

/*
                                                                
 */
#define MNTPATHLEN		(1024)

/*
                      
 */
#define encode_dirpath_sz	(1 + XDR_QUADLEN(MNTPATHLEN))
#define MNT_status_sz		(1)
#define MNT_fhs_status_sz	(1)
#define MNT_fhandle_sz		XDR_QUADLEN(NFS2_FHSIZE)
#define MNT_fhandle3_sz		(1 + XDR_QUADLEN(NFS3_FHSIZE))
#define MNT_authflav3_sz	(1 + NFS_MAX_SECFLAVORS)

/*
                                
 */
#define MNT_enc_dirpath_sz	encode_dirpath_sz
#define MNT_dec_mountres_sz	(MNT_status_sz + MNT_fhandle_sz)
#define MNT_dec_mountres3_sz	(MNT_status_sz + MNT_fhandle_sz + \
				 MNT_authflav3_sz)

/*
                                   
 */
enum {
	MOUNTPROC_NULL		= 0,
	MOUNTPROC_MNT		= 1,
	MOUNTPROC_DUMP		= 2,
	MOUNTPROC_UMNT		= 3,
	MOUNTPROC_UMNTALL	= 4,
	MOUNTPROC_EXPORT	= 5,
};

/*
                                   
 */
enum {
	MOUNTPROC3_NULL		= 0,
	MOUNTPROC3_MNT		= 1,
	MOUNTPROC3_DUMP		= 2,
	MOUNTPROC3_UMNT		= 3,
	MOUNTPROC3_UMNTALL	= 4,
	MOUNTPROC3_EXPORT	= 5,
};

static const struct rpc_program mnt_program;

/*
                                                  
 */
enum mountstat {
	MNT_OK			= 0,
	MNT_EPERM		= 1,
	MNT_ENOENT		= 2,
	MNT_EACCES		= 13,
	MNT_EINVAL		= 22,
};

static struct {
	u32 status;
	int errno;
} mnt_errtbl[] = {
	{ .status = MNT_OK,			.errno = 0,		},
	{ .status = MNT_EPERM,			.errno = -EPERM,	},
	{ .status = MNT_ENOENT,			.errno = -ENOENT,	},
	{ .status = MNT_EACCES,			.errno = -EACCES,	},
	{ .status = MNT_EINVAL,			.errno = -EINVAL,	},
};

/*
                                     
 */
enum mountstat3 {
	MNT3_OK			= 0,		/*          */
	MNT3ERR_PERM		= 1,		/*           */
	MNT3ERR_NOENT		= 2,		/*                           */
	MNT3ERR_IO		= 5,		/*           */
	MNT3ERR_ACCES		= 13,		/*                   */
	MNT3ERR_NOTDIR		= 20,		/*                 */
	MNT3ERR_INVAL		= 22,		/*                  */
	MNT3ERR_NAMETOOLONG	= 63,		/*                   */
	MNT3ERR_NOTSUPP		= 10004,	/*                         */
	MNT3ERR_SERVERFAULT	= 10006,	/*                         */
};

static struct {
	u32 status;
	int errno;
} mnt3_errtbl[] = {
	{ .status = MNT3_OK,			.errno = 0,		},
	{ .status = MNT3ERR_PERM,		.errno = -EPERM,	},
	{ .status = MNT3ERR_NOENT,		.errno = -ENOENT,	},
	{ .status = MNT3ERR_IO,			.errno = -EIO,		},
	{ .status = MNT3ERR_ACCES,		.errno = -EACCES,	},
	{ .status = MNT3ERR_NOTDIR,		.errno = -ENOTDIR,	},
	{ .status = MNT3ERR_INVAL,		.errno = -EINVAL,	},
	{ .status = MNT3ERR_NAMETOOLONG,	.errno = -ENAMETOOLONG,	},
	{ .status = MNT3ERR_NOTSUPP,		.errno = -ENOTSUPP,	},
	{ .status = MNT3ERR_SERVERFAULT,	.errno = -EREMOTEIO,	},
};

struct mountres {
	int errno;
	struct nfs_fh *fh;
	unsigned int *auth_count;
	rpc_authflavor_t *auth_flavors;
};

struct mnt_fhstatus {
	u32 status;
	struct nfs_fh *fh;
};

/* 
                                                                    
                                            
  
                                                                     
 */
int nfs_mount(struct nfs_mount_request *info)
{
	struct mountres	result = {
		.fh		= info->fh,
		.auth_count	= info->auth_flav_len,
		.auth_flavors	= info->auth_flavs,
	};
	struct rpc_message msg	= {
		.rpc_argp	= info->dirpath,
		.rpc_resp	= &result,
	};
	struct rpc_create_args args = {
		.net		= info->net,
		.protocol	= info->protocol,
		.address	= info->sap,
		.addrsize	= info->salen,
		.servername	= info->hostname,
		.program	= &mnt_program,
		.version	= info->version,
		.authflavor	= RPC_AUTH_UNIX,
	};
	struct rpc_clnt		*mnt_clnt;
	int			status;

	dprintk("NFS: sending MNT request for %s:%s\n",
		(info->hostname ? info->hostname : "server"),
			info->dirpath);

	if (info->noresvport)
		args.flags |= RPC_CLNT_CREATE_NONPRIVPORT;

	mnt_clnt = rpc_create(&args);
	if (IS_ERR(mnt_clnt))
		goto out_clnt_err;

	if (info->version == NFS_MNT3_VERSION)
		msg.rpc_proc = &mnt_clnt->cl_procinfo[MOUNTPROC3_MNT];
	else
		msg.rpc_proc = &mnt_clnt->cl_procinfo[MOUNTPROC_MNT];

	status = rpc_call_sync(mnt_clnt, &msg, 0);
	rpc_shutdown_client(mnt_clnt);

	if (status < 0)
		goto out_call_err;
	if (result.errno != 0)
		goto out_mnt_err;

	dprintk("NFS: MNT request succeeded\n");
	status = 0;

out:
	return status;

out_clnt_err:
	status = PTR_ERR(mnt_clnt);
	dprintk("NFS: failed to create MNT RPC client, status=%d\n", status);
	goto out;

out_call_err:
	dprintk("NFS: MNT request failed, status=%d\n", status);
	goto out;

out_mnt_err:
	dprintk("NFS: MNT server returned result %d\n", result.errno);
	status = result.errno;
	goto out;
}

/* 
                                                                  
                                             
  
                                                                    
           
 */
void nfs_umount(const struct nfs_mount_request *info)
{
	static const struct rpc_timeout nfs_umnt_timeout = {
		.to_initval = 1 * HZ,
		.to_maxval = 3 * HZ,
		.to_retries = 2,
	};
	struct rpc_create_args args = {
		.net		= info->net,
		.protocol	= IPPROTO_UDP,
		.address	= info->sap,
		.addrsize	= info->salen,
		.timeout	= &nfs_umnt_timeout,
		.servername	= info->hostname,
		.program	= &mnt_program,
		.version	= info->version,
		.authflavor	= RPC_AUTH_UNIX,
		.flags		= RPC_CLNT_CREATE_NOPING,
	};
	struct rpc_message msg	= {
		.rpc_argp	= info->dirpath,
	};
	struct rpc_clnt *clnt;
	int status;

	if (info->noresvport)
		args.flags |= RPC_CLNT_CREATE_NONPRIVPORT;

	clnt = rpc_create(&args);
	if (IS_ERR(clnt))
		goto out_clnt_err;

	dprintk("NFS: sending UMNT request for %s:%s\n",
		(info->hostname ? info->hostname : "server"), info->dirpath);

	if (info->version == NFS_MNT3_VERSION)
		msg.rpc_proc = &clnt->cl_procinfo[MOUNTPROC3_UMNT];
	else
		msg.rpc_proc = &clnt->cl_procinfo[MOUNTPROC_UMNT];

	status = rpc_call_sync(clnt, &msg, 0);
	rpc_shutdown_client(clnt);

	if (unlikely(status < 0))
		goto out_call_err;

	return;

out_clnt_err:
	dprintk("NFS: failed to create UMNT RPC client, status=%ld\n",
			PTR_ERR(clnt));
	return;

out_call_err:
	dprintk("NFS: UMNT request failed, status=%d\n", status);
}

/*
                                        
 */

static void encode_mntdirpath(struct xdr_stream *xdr, const char *pathname)
{
	const u32 pathname_len = strlen(pathname);
	__be32 *p;

	BUG_ON(pathname_len > MNTPATHLEN);
	p = xdr_reserve_space(xdr, 4 + pathname_len);
	xdr_encode_opaque(p, pathname, pathname_len);
}

static void mnt_xdr_enc_dirpath(struct rpc_rqst *req, struct xdr_stream *xdr,
				const char *dirpath)
{
	encode_mntdirpath(xdr, dirpath);
}

/*
                                                                      
                                                                     
                                                                   
         
  
                                                                     
                                                                  
 */
static int decode_status(struct xdr_stream *xdr, struct mountres *res)
{
	unsigned int i;
	u32 status;
	__be32 *p;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return -EIO;
	status = be32_to_cpup(p);

	for (i = 0; i < ARRAY_SIZE(mnt_errtbl); i++) {
		if (mnt_errtbl[i].status == status) {
			res->errno = mnt_errtbl[i].errno;
			return 0;
		}
	}

	dprintk("NFS: unrecognized MNT status code: %u\n", status);
	res->errno = -EACCES;
	return 0;
}

static int decode_fhandle(struct xdr_stream *xdr, struct mountres *res)
{
	struct nfs_fh *fh = res->fh;
	__be32 *p;

	p = xdr_inline_decode(xdr, NFS2_FHSIZE);
	if (unlikely(p == NULL))
		return -EIO;

	fh->size = NFS2_FHSIZE;
	memcpy(fh->data, p, NFS2_FHSIZE);
	return 0;
}

static int mnt_xdr_dec_mountres(struct rpc_rqst *req,
				struct xdr_stream *xdr,
				struct mountres *res)
{
	int status;

	status = decode_status(xdr, res);
	if (unlikely(status != 0 || res->errno != 0))
		return status;
	return decode_fhandle(xdr, res);
}

static int decode_fhs_status(struct xdr_stream *xdr, struct mountres *res)
{
	unsigned int i;
	u32 status;
	__be32 *p;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return -EIO;
	status = be32_to_cpup(p);

	for (i = 0; i < ARRAY_SIZE(mnt3_errtbl); i++) {
		if (mnt3_errtbl[i].status == status) {
			res->errno = mnt3_errtbl[i].errno;
			return 0;
		}
	}

	dprintk("NFS: unrecognized MNT3 status code: %u\n", status);
	res->errno = -EACCES;
	return 0;
}

static int decode_fhandle3(struct xdr_stream *xdr, struct mountres *res)
{
	struct nfs_fh *fh = res->fh;
	u32 size;
	__be32 *p;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return -EIO;

	size = be32_to_cpup(p);
	if (size > NFS3_FHSIZE || size == 0)
		return -EIO;

	p = xdr_inline_decode(xdr, size);
	if (unlikely(p == NULL))
		return -EIO;

	fh->size = size;
	memcpy(fh->data, p, size);
	return 0;
}

static int decode_auth_flavors(struct xdr_stream *xdr, struct mountres *res)
{
	rpc_authflavor_t *flavors = res->auth_flavors;
	unsigned int *count = res->auth_count;
	u32 entries, i;
	__be32 *p;

	if (*count == 0)
		return 0;

	p = xdr_inline_decode(xdr, 4);
	if (unlikely(p == NULL))
		return -EIO;
	entries = be32_to_cpup(p);
	dprintk("NFS: received %u auth flavors\n", entries);
	if (entries > NFS_MAX_SECFLAVORS)
		entries = NFS_MAX_SECFLAVORS;

	p = xdr_inline_decode(xdr, 4 * entries);
	if (unlikely(p == NULL))
		return -EIO;

	if (entries > *count)
		entries = *count;

	for (i = 0; i < entries; i++) {
		flavors[i] = be32_to_cpup(p++);
		dprintk("NFS:   auth flavor[%u]: %d\n", i, flavors[i]);
	}
	*count = i;

	return 0;
}

static int mnt_xdr_dec_mountres3(struct rpc_rqst *req,
				 struct xdr_stream *xdr,
				 struct mountres *res)
{
	int status;

	status = decode_fhs_status(xdr, res);
	if (unlikely(status != 0 || res->errno != 0))
		return status;
	status = decode_fhandle3(xdr, res);
	if (unlikely(status != 0)) {
		res->errno = -EBADHANDLE;
		return 0;
	}
	return decode_auth_flavors(xdr, res);
}

static struct rpc_procinfo mnt_procedures[] = {
	[MOUNTPROC_MNT] = {
		.p_proc		= MOUNTPROC_MNT,
		.p_encode	= (kxdreproc_t)mnt_xdr_enc_dirpath,
		.p_decode	= (kxdrdproc_t)mnt_xdr_dec_mountres,
		.p_arglen	= MNT_enc_dirpath_sz,
		.p_replen	= MNT_dec_mountres_sz,
		.p_statidx	= MOUNTPROC_MNT,
		.p_name		= "MOUNT",
	},
	[MOUNTPROC_UMNT] = {
		.p_proc		= MOUNTPROC_UMNT,
		.p_encode	= (kxdreproc_t)mnt_xdr_enc_dirpath,
		.p_arglen	= MNT_enc_dirpath_sz,
		.p_statidx	= MOUNTPROC_UMNT,
		.p_name		= "UMOUNT",
	},
};

static struct rpc_procinfo mnt3_procedures[] = {
	[MOUNTPROC3_MNT] = {
		.p_proc		= MOUNTPROC3_MNT,
		.p_encode	= (kxdreproc_t)mnt_xdr_enc_dirpath,
		.p_decode	= (kxdrdproc_t)mnt_xdr_dec_mountres3,
		.p_arglen	= MNT_enc_dirpath_sz,
		.p_replen	= MNT_dec_mountres3_sz,
		.p_statidx	= MOUNTPROC3_MNT,
		.p_name		= "MOUNT",
	},
	[MOUNTPROC3_UMNT] = {
		.p_proc		= MOUNTPROC3_UMNT,
		.p_encode	= (kxdreproc_t)mnt_xdr_enc_dirpath,
		.p_arglen	= MNT_enc_dirpath_sz,
		.p_statidx	= MOUNTPROC3_UMNT,
		.p_name		= "UMOUNT",
	},
};


static const struct rpc_version mnt_version1 = {
	.number		= 1,
	.nrprocs	= ARRAY_SIZE(mnt_procedures),
	.procs		= mnt_procedures,
};

static const struct rpc_version mnt_version3 = {
	.number		= 3,
	.nrprocs	= ARRAY_SIZE(mnt3_procedures),
	.procs		= mnt3_procedures,
};

static const struct rpc_version *mnt_version[] = {
	NULL,
	&mnt_version1,
	NULL,
	&mnt_version3,
};

static struct rpc_stat mnt_stats;

static const struct rpc_program mnt_program = {
	.name		= "mount",
	.number		= NFS_MNT_PROGRAM,
	.nrvers		= ARRAY_SIZE(mnt_version),
	.version	= mnt_version,
	.stats		= &mnt_stats,
};
