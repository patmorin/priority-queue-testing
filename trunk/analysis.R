library("ggplot2")
library("nnls")

result_sets <- c(
    "eager/acyc_pos",
    "eager/grid_phard",
    "eager/grid_slong",
    "eager/grid_ssquare",
    "eager/grid_ssquare_s",
    "eager/nix",
    "eager/pq_dcr_min_one_long",
    "eager/pq_dcr_min_one_medium",
    "eager/pq_dcr_min_one_short",
    "eager/pq_dcr_min_few_long",
    "eager/pq_dcr_min_few_medium",
    "eager/pq_dcr_min_few_short",
    "eager/pq_dcr_min_many_long",
    "eager/pq_dcr_min_many_medium",
    "eager/pq_dcr_min_many_short",
    "eager/pq_dcr_one_long",
    "eager/pq_dcr_one_medium",
    "eager/pq_dcr_one_short",
    "eager/pq_dcr_few_long",
    "eager/pq_dcr_few_medium",
    "eager/pq_dcr_few_short",
    "eager/pq_dcr_many_long",
    "eager/pq_dcr_many_medium",
    "eager/pq_dcr_many_short",
    "eager/pq_id_long",
    "eager/pq_id_medium",
    "eager/pq_id_short",
    "eager/pq_sort",
    "eager/rand_1_4",
    "eager/rand_4",
    "eager/spbad_dense",
    "eager/spbad_sparse",
    "eager/usa"
)

queue_sets <- c(
    "eager/binomial",
    "eager/explicit_2",
    "eager/explicit_4",
    "eager/explicit_8",
    "eager/explicit_16",
    "eager/fibonacci",
    "eager/implicit_2",
    "eager/implicit_4",
    "eager/implicit_8",
    "eager/implicit_16",
    "eager/implicit_simple_2",
    "eager/implicit_simple_4",
    "eager/implicit_simple_8",
    "eager/implicit_simple_16",
    "eager/pairing",
    "eager/quake",
    "eager/rank_pairing_t1",
    "eager/rank_pairing_t2",
    "eager/rank_relaxed_weak",
    "eager/strict_fibonacci",
    "eager/violation"
)

shrink <- function( results, trials )
{
    if( trials == 1 )
        return( results )
    squashed <- results[0,]
    levels( squashed$test ) <- levels( results$test )
    for( i in seq( dim(results)[1]/trials ) )
    {
        end <- trials * i
        start <- end - trials + 1
        squashed[i,1:2] <- results[start,1:2]
        squashed[i,3:17] <- colMeans( results[start:end,3:17] )
        squashed[i,3:11] <- round( squashed[i,3:11] )
        squashed[i,13:14] <- round( squashed[i,13:14] )
        squashed[i,16] <- round( squashed[i,16] )
    }

    levs <- levels( squashed[,2] )
    for( i in seq( length( levs ) ) )
    {
        levs[i] <- substr( levs[i], 1, nchar( levs[i] ) - 2 )
    }
    levels( squashed[,2] ) <- levs

    return( squashed )
}

center <- function( results, num.heaps )
{
    test.count <- dim(results)[1] / (num.heaps+1)
    dummy <- results[(test.count+1):(2*test.count),]
    the.offset <- do.call("rbind", replicate(num.heaps+1, dummy, simplify = FALSE))
    c.time <- results$time - the.offset$time
    c.inst <- results$inst - the.offset$inst
    c.l1_rd <- results$l1_rd - the.offset$l1_rd
    c.l1_wr <- results$l1_wr - the.offset$l1_wr
    c.l3_rd <- results$l3_rd - the.offset$l3_rd
    c.l3_wr <- results$l3_wr - the.offset$l3_wr
    c.branch <- results$branch - the.offset$branch
    centered <- data.frame(queue=results$queue, file=results$file, max_size=results$max_size, avg_size=results$avg_size, ins=results$ins, dmn=results$dmn, dcr=results$dcr, time=c.time, inst=c.inst, l1_rd=c.l1_rd, l1_wr=c.l1_wr, l1_miss=results$l1_miss, l3_rd=c.l3_rd, l3_wr=c.l3_wr, l3_miss=results$l3_miss, branch=c.branch, mispredict=results$mispredict)
    proper <- centered[c(1:test.count,(2*test.count+1):(dim(centered)[1])),]

    # fix levels of queue
    new.levels <- levels( proper$queue )[levels( proper$queue ) != "dummy"]
    for( i in seq( length( levels( proper$queue ) ) ) )
    {
        if( levels( proper$queue )[i] == "dummy" )
            match <- i
    }
    queues <- as.numeric( proper$queue )
    for( i in seq( length( proper$queue ) ) )
    {
        if( queues[i] > match )
            queues[i] <- queues[i] - 1
    }
    proper$queue <- as.factor( queues )
    levels( proper$queue ) <- new.levels

    return( proper )
}

center.queue <- function( results, is.short )
{
    if( is.short )
        the.offset <- read.csv("results/eager/dummy.short.csv")
    else
        the.offset <- read.csv("results/eager/dummy.csv")
    c.time <- results$time - the.offset$time
    c.inst <- results$inst - the.offset$inst
    c.l1_rd <- results$l1_rd - the.offset$l1_rd
    c.l1_wr <- results$l1_wr - the.offset$l1_wr
    c.l3_rd <- results$l3_rd - the.offset$l3_rd
    c.l3_wr <- results$l3_wr - the.offset$l3_wr
    c.branch <- results$branch - the.offset$branch
    centered <- data.frame(queue=results$queue, file=results$file, max_size=results$max_size, avg_size=results$avg_size, ins=results$ins, dmn=results$dmn, dcr=results$dcr, time=c.time, inst=c.inst, l1_rd=c.l1_rd, l1_wr=c.l1_wr, l1_miss=results$l1_miss, l3_rd=c.l3_rd, l3_wr=c.l3_wr, l3_miss=results$l3_miss, branch=c.branch, mispredict=results$mispredict)

    return( centered )
}

augment <- function( results )
{
    log.factor <- log(results$avg_size,2)
    combined <- round( results$ins + ( log.factor * results$dmn ) + results$dcr )
    count_l1 <- results$l1_miss * ( results$l1_rd + results$l1_wr )
    count_l3 <- results$l3_miss * ( results$l3_rd + results$l3_wr )
    count_br <- results$mispredict * results$branch
    new.results <- data.frame(queue=results$queue, file=results$file, max_size=results$max_size, avg_size=results$avg_size, ins=results$ins, dmn=results$dmn, dcr=results$dcr, time=results$time, inst=results$inst, l1_rd=results$l1_rd, l1_wr=results$l1_wr, l1_miss=results$l1_miss, l3_rd=results$l3_rd, l3_wr=results$l3_wr, l3_miss=results$l3_miss, branch=results$branch, mispredict=results$mispredict, combined=combined, count_l1=count_l1, count_l3=count_l3, count_br=count_br )
    return( new.results )
}

relabel <- function( results )
{
    results.ordered <- results
    new.lev <- vector()
    lev <- levels( results$queue )
    N <- length(lev)
    index <- vector()
    t <- vector()
    for( i in seq(N) )
    {
        qu <- lev[i]
        set <- subset( results, queue == qu )
        index[i] <- i
        t[i] <- tail( set$time, 1 )
    }
    f <- data.frame(lev=lev,index=index,t=t)
    f <- f[order(-f$t),]
    f <- data.frame(lev=f$lev,index=f$index,t=f$t)
    new.index <- f$index
    qu <- as.numeric(results.ordered$queue)
    for( i in seq(N) )
    {
        qu[qu == new.index[i]] <- i + N
    }

    results.ordered$queue <- as.factor(qu - N)
    levels( results.ordered$queue ) <- lev[new.index]

    results.labeled <- results.ordered
    lev <- levels( results.labeled$queue )
    new.lev <- vector()
    for( i in seq(N) )
    {
        qu <- lev[i]
        set <- subset( results, queue == qu )
        l1 <- signif(tail( set$l1_miss, 1 ),3)
        mispred <- signif(tail( set$mispredict, 1 ),3)
        q.new <- paste( qu, " (", l1, ", ", mispred, ")", sep="" )
        new.lev[i] <- q.new
    }
    levels( results.labeled$queue ) <- new.lev

    results.sorted <- results.labeled[order(as.numeric(results.labeled$queue)),]

    return( results.sorted )
}

make.chart <- function( results, out )
{
    the.title <- substr( out, 1, nchar( out ) - 4 )
    postscript(file=paste("graphs/",out,sep=""),width=10,height=5,paper="special")
    p <- ggplot(results,aes(combined,time,group=queue,color=queue,shape=queue)) + scale_y_log10() + scale_x_log10() + geom_point(size=1.0) + geom_line(size=0.05) + scale_shape_manual(values=unique(as.numeric(results$queue))) + labs( title=the.title )

    print( p )
    aux <- dev.off()
}

regression.table <- function( results, filename, num.heaps )
{
    tests <- length( results ) / num.heaps
    queues <- levels( results$queue )
    time <- vector()
    inst <- vector()
    l3_rd <- vector()
    l3_wr <- vector()
    l1_rd <- vector()
    l1_wr <- vector()
    branch <- vector()
    l1_miss <- vector()
    l3_miss <- vector()
    mispredict <- vector()

    for( i in seq(num.heaps) )
    {
        set <- subset( results, queue == queues[i] )
        time[i] <- lm(set$time~set$combined)$coefficients[2]
        inst[i] <- lm(set$inst~set$combined)$coefficients[2]
        l1_rd[i] <- lm(set$l1_rd~set$combined)$coefficients[2]
        l1_wr[i] <- lm(set$l1_wr~set$combined)$coefficients[2]
        l3_rd[i] <- lm(set$l3_rd~set$combined)$coefficients[2]
        l3_wr[i] <- lm(set$l3_wr~set$combined)$coefficients[2]
        branch[i] <- lm(set$branch~set$combined)$coefficients[2]
        #l1_miss[i] <- tail( set$l1_miss, 1 )
        #l3_miss[i] <- tail( set$l3_miss, 1 )
        #mispredict[i] <- tail( set$mispredict, 1 )
        l1_miss[i] <- lm(set$count_l1~set$combined)$coefficients[2]
        l3_miss[i] <- lm(set$count_l3~set$combined)$coefficients[2]
        mispredict[i] <- lm(set$count_br~set$combined)$coefficients[2]
    }

    stats <- matrix(0,1,3)
    stats[1,1] <- tail( set$ins, 1 )
    stats[1,2] <- tail( set$dmn, 1 )
    stats[1,3] <- tail( set$dcr, 1 )
    stats <- stats / min( stats )
    max_size <- max( set$max_size )
    avg_size <- max( set$avg_size )

    sink( paste( "tables/", filename, sep="" ) )
    options(digits=3,width=500)
    print( paste( "Heap Size [ max, avg ] -> [", paste( max_size, avg_size, sep=", " ), "]" ) )
    print( paste( "Ratio of Operations [ ins, dmn, dcr ] -> [", paste( stats[1,1], stats[1,2], stats[1,3], sep=", " ), "]" ) )
    cat("\n")

    f <- data.frame( queue=as.factor(seq(num.heaps)), time=time, inst=inst, l1_rd=l1_rd, l1_wr=l1_wr, l3_rd=l3_rd, l3_wr=l3_wr, branch=branch, l1_miss=l1_miss, l3_miss=l3_miss, mispredict=mispredict )
    levels( f$queue ) <- queues
    f <- f[order(-f$time),]

    print( f )
    cat("\n")

    constrained.regression( results )
    cat("\n")

    numbers <- data.frame(time=results$time, inst=results$inst, l1_rd=results$l1_rd, l1_wr=results$l1_wr, l1_miss=results$l1_miss, l3_rd=results$l3_rd, l3_wr=results$l3_wr, l3_miss=results$l3_miss, branch=results$branch, mispredict=results$mispredict, count_l1=results$count_l1, count_l3=results$count_l3, count_br=results$count_br )
    cat( "Correlations:\n")
    print( cor( numbers ) )

    sink()

    #return( f )
}

post.process.result <- function( name )
{
    result.full <- read.csv( paste( "results/", name, ".csv", sep="" ) )
    if( name == "usa" )
        trials <- 1
    else
        trials <- 3
    if( name == "eager/pq_sort" || name == "eager/pq_id_short" || name == "eager/pq_id_medium" || name == "eager/pq_id_long" )
        num.heaps <- 21
    else
        num.heaps <- 17

    result.shrunk <- shrink( result.full, trials )
    result.centered <- center( result.shrunk, num.heaps )
    result.augmented <- augment( result.centered )
    result.final <- relabel( result.augmented )

    regression.table( result.augmented, paste( name, ".txt", sep="" ), num.heaps )
}

post.process.queue <- function( name )
{
    is.short <- ( name == "eager/implicit_simple_2" || name == "eager/implicit_simple_4" || name == "eager/implicit_simple_8" || name == "eager/implicit_simple_16" )
    result.raw <- read.csv( paste( "results/", name, ".csv", sep="" ) )
    result.centered <- center.queue( result.raw, is.short )
    results <- augment( result.centered )

    sink( paste( "tables/", name, ".txt", sep="" ) )
    options(digits=3,width=500)

    constrained.regression( results )
    cat("\n")

    numbers <- data.frame(time=results$time, inst=results$inst, l1_rd=results$l1_rd, l1_wr=results$l1_wr, l1_miss=results$l1_miss, l3_rd=results$l3_rd, l3_wr=results$l3_wr, l3_miss=results$l3_miss, branch=results$branch, mispredict=results$mispredict, count_l1=results$count_l1, count_l3=results$count_l3, count_br=results$count_br )
    cat( "Correlations:\n" )
    print( cor( numbers ) )

    sink()
}

constrained.regression <- function( results )
{
    X <- as.matrix( results[,c("l1_rd","l1_wr","l3_rd","l3_wr","branch","count_l1","count_l3","count_br")] )
    Y <- results$time

    S <- nnls( X, Y )

    normed <- coef(S)/max(coef(S))

    cat( paste( 'l1_rd:    ', normed[1], "\n", sep="" ) )
    cat( paste( 'l1_wr:    ', normed[2], "\n", sep="" ) )
    cat( paste( 'l3_rd:    ', normed[3], "\n", sep="" ) )
    cat( paste( 'l3_wr:    ', normed[4], "\n", sep="" ) )
    cat( paste( 'branch:   ', normed[5], "\n", sep="" ) )
    cat( paste( 'count_l1: ', normed[6], "\n", sep="" ) )
    cat( paste( 'count_l3: ', normed[7], "\n", sep="" ) )
    cat( paste( 'count_br: ', normed[8], "\n", sep="" ) )
}

process.all <- function()
{
    for( set in result_sets )
    {
        post.process.result(set)
    }

    for( set in queue_sets )
    {
        post.process.queue(set)
    }
}
